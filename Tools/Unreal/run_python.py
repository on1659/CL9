"""Execute a reviewed script in the already running CL9 UE5.8 editor."""
import json
import sys
import time
from pathlib import Path

sys.path.insert(0, 'D:/Program Files/Epic Games/UE_5.8/Engine/Plugins/Experimental/PythonScriptPlugin/Content/Python')
import remote_execution

remote = remote_execution.RemoteExecution()
remote.start()
try:
    deadline = time.monotonic() + 5
    nodes = []
    while time.monotonic() < deadline:
        nodes = [n for n in remote.remote_nodes if n.get('project_name') == 'CL9']
        if nodes:
            break
        time.sleep(0.1)
    if len(nodes) != 1:
        raise RuntimeError(f'Expected one CL9 editor: {remote.remote_nodes}')
    print(json.dumps(nodes, ensure_ascii=False), flush=True)
    remote.open_command_connection(nodes[0]['node_id'])
    script = Path(sys.argv[1]).resolve()
    result = remote.run_command(str(script).replace('\\','/'), raise_on_failure=False)
    print(json.dumps(result,ensure_ascii=False,indent=2))
    if not result['success']:
        sys.exit(1)
finally:
    remote.stop()
