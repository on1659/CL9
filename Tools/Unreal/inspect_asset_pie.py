import json
from pathlib import Path
import unreal
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
report={'engine':unreal.SystemLibrary.get_engine_version(),'worlds':[]}
for world in worlds:
    record={'world':world.get_path_name(),'characters':[],'items':[]}
    for a in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.Actor):
        cls=a.get_class().get_name()
        if cls=='BP_CL9Character_C':
            record['characters'].append({'name':a.get_name(),'location':list(a.get_actor_location().to_tuple()),
                'role':str(a.get_local_role()),'controller':str(a.get_controller()),
                'rpc_methods':[n for n in dir(a) if 'pick' in n or 'swap' in n or 'held' in n or 'throw' in n]})
        if cls=='BP_CL9Item_C':
            sm=a.get_component_by_class(unreal.StaticMeshComponent)
            origin,extent,radius=unreal.SystemLibrary.get_component_bounds(sm)
            sphere=a.get_component_by_class(unreal.SphereComponent)
            record['items'].append({'name':a.get_name(),'alias':str(a.get_editor_property('alias')),
                'sub_alias':str(a.get_editor_property('sub_alias')),'location':list(a.get_actor_location().to_tuple()),
                'mesh':sm.static_mesh.get_path_name(),'mesh_scale':list(sm.get_world_scale().to_tuple()),
                'mesh_bottom_z':origin.z-extent.z,'sphere_radius':sphere.get_scaled_sphere_radius(),
                'attached_to':str(a.get_attach_parent_actor())})
    report['worlds'].append(record)
Path(unreal.Paths.project_saved_dir(),'asset-pie-current.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
