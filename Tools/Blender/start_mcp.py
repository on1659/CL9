"""Start the installed Blender MCP addon in this newly launched Blender session."""

import traceback
from datetime import datetime
from pathlib import Path

import addon_utils
import bpy

LOG = Path(__file__).resolve().parents[2] / "Saved" / "BlenderMCP" / "startup.log"
LOG.parent.mkdir(parents=True, exist_ok=True)


def log(message):
    with LOG.open("a", encoding="utf-8") as stream:
        stream.write(f"{datetime.now().isoformat()} {message}\n")


log("Startup script loaded")
attempts = 0

def start_mcp():
    global attempts
    attempts += 1
    try:
        if not hasattr(bpy.types.Scene, "blendermcp_port"):
            candidates = [
                module for module in addon_utils.modules(refresh=True)
                if module.bl_info.get("name") in ("MCP for Blender", "Blender MCP", "BlenderMCP")
            ]
            if not candidates:
                raise RuntimeError("Blender MCP addon is not installed. Install addon.py first.")
            addon_utils.enable(candidates[0].__name__, default_set=False, persistent=True)

        bpy.context.scene.blendermcp_port = 9876
        server = getattr(bpy.types, "blendermcp_server", None)
        if not server or not server.running:
            bpy.ops.blendermcp.start_server()
        server = getattr(bpy.types, "blendermcp_server", None)
        if not server or not server.running:
            raise RuntimeError("Blender MCP failed to start on port 9876.")
        # Startup-file loading can reset the scene flag after addon registration.
        bpy.context.scene.blendermcp_server_running = True
        for screen in bpy.data.screens:
            for area in screen.areas:
                area.tag_redraw()
        print("CL9: Blender MCP is running on port 9876.")
        log("Connected: Blender MCP running on port 9876")
    except Exception:
        traceback.print_exc()
        log(traceback.format_exc())
        if attempts < 5:
            return 2.0

        def draw_error(self, context):
            self.layout.label(text="MCP startup failed. See Window > Toggle System Console.")

        bpy.context.window_manager.popup_menu(draw_error, title="CL9 Blender MCP", icon="ERROR")
    return None


bpy.app.timers.register(start_mcp, first_interval=2.0, persistent=True)
