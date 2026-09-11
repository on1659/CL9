import unreal,json
es=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
rows=[]
for a in es.get_all_level_actors():
    if 'Jar' not in a.get_class().get_name():continue
    row={'name':a.get_name(),'class':a.get_class().get_path_name(),'location':a.get_actor_location().to_tuple(),'scale':a.get_actor_scale3d().to_tuple(),'components':[]}
    for c in a.get_components_by_class(unreal.SceneComponent):
        cr={'name':c.get_name(),'class':c.get_class().get_name(),'relative_location':c.get_editor_property('relative_location').to_tuple(),'relative_scale':c.get_editor_property('relative_scale3d').to_tuple()}
        if isinstance(c,unreal.PrimitiveComponent):
            o,e,r=unreal.SystemLibrary.get_component_bounds(c)
            cr.update(bounds_center=o.to_tuple(),dimensions=[v*2 for v in e.to_tuple()],collision=str(c.get_collision_profile_name()))
        if isinstance(c,unreal.StaticMeshComponent):cr['mesh']=c.static_mesh.get_path_name() if c.static_mesh else None
        row['components'].append(cr)
    rows.append(row)
from pathlib import Path
Path('D:/Work/unreal/CL9/docs/asset/evidence/019-jar-consumer.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
print(json.dumps(rows))
