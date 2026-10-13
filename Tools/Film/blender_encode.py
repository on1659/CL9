"""Blender 내장 FFmpeg 로 JPEG 프레임 묶음을 H.264 MP4 로 — encode.py 가 ffmpeg 를 못 찾을 때만 쓴다.
쓰기: blender -b --factory-startup --python blender_encode.py -- <frames_dir> <out.mp4> <fps>
"""
import os
import sys

import bpy

argv = sys.argv[sys.argv.index('--') + 1:]
frames_dir, out_path, fps = argv[0], argv[1], int(argv[2])
files = sorted(f for f in os.listdir(frames_dir) if f.lower().endswith('.jpg'))

scene = bpy.context.scene
scene.render.resolution_x = 1280
scene.render.resolution_y = 720
scene.render.resolution_percentage = 100
scene.render.fps = fps
scene.render.fps_base = 1.0
scene.frame_start = 1
scene.frame_end = len(files)

se = scene.sequence_editor_create()
coll = se.strips if hasattr(se, 'strips') else se.sequences
strip = coll.new_image(name='frames', filepath=os.path.join(frames_dir, files[0]), channel=1, frame_start=1)
for f in files[1:]:
    strip.elements.append(f)

img = scene.render.image_settings
try:
    img.media_type = 'VIDEO'
except Exception:
    pass
img.file_format = 'FFMPEG'
ff = scene.render.ffmpeg
ff.format = 'MPEG4'
ff.codec = 'H264'
try:
    ff.constant_rate_factor = 'HIGH'
except Exception:
    pass
ff.audio_codec = 'NONE'
scene.render.use_sequencer = True
scene.render.filepath = out_path
bpy.ops.render.render(animation=True)
print('VIDEO OK', out_path, len(files), 'frames at', fps, 'fps')
