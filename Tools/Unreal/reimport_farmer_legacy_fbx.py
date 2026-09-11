"""Use the FBX options actually requested, restoring the editor's Interchange flag afterward."""
import unreal
from pathlib import Path
e=Path(unreal.Paths.project_dir(),'docs/asset/evidence/024-farmer-imports.json');backup=e.with_name('024-farmer-imports-before-colour-fix.json');assert not backup.exists();backup.write_text(e.read_text(),encoding='utf-8')
flag='Interchange.FeatureFlags.Import.FBX';old=unreal.SystemLibrary.get_console_variable_int_value(flag)
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
try:
 unreal.SystemLibrary.execute_console_command(w,flag+' 0');REIMPORT_OWNED=True
 exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_farmer_a.py',encoding='utf-8').read(),'import_farmer_a.py','exec'))
finally:unreal.SystemLibrary.execute_console_command(w,flag+' '+str(old))
print('Restored Interchange FBX flag to',old)
