"""Real item cheat spawn and measurements in available PIE worlds. No synthetic meshes."""
import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
assert worlds,'Requires active PIE'
host=next(w for w in worlds if 'UEDPIE_0_' in w.get_path_name())
def items(w):
    return [a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor)
            if a.get_class().get_name()=='BP_CL9Item_C' and str(a.get_editor_property('alias'))=='meat']
root=Path(unreal.Paths.project_dir()).resolve()
if globals().get('MODE')!='measure':
    pc=unreal.GameplayStatics.get_player_controller(host,0)
    before={a.get_path_name() for a in items(host)}
    for number in (6,7,8):unreal.SystemLibrary.execute_console_command(host,'cl9.additem '+str(number),pc)
    spawned=[a for a in items(host) if a.get_path_name() not in before]
    assert len(spawned)==3,[(a.get_name(),str(a.get_editor_property('sub_alias'))) for a in spawned]
    base=unreal.GameplayStatics.get_player_pawn(host,0).get_actor_location()
    for i,a in enumerate(spawned):
        a.set_actor_location(unreal.Vector(base.x+200,base.y+(i-1)*100,base.z+20),False,True)
    print('Spawned real three meat rows; allow physics to run before measurement')
else:
    report={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'engine':unreal.SystemLibrary.get_engine_version(),
            'scenario':'Lvl_Flat / cl9.additem 6,7,8 / physical simulation','world_count':len(worlds),
            'scope':'Mesh selection, scale and floor gap. No carry/cooking gameplay claim.','worlds':[]}
    for w in worlds:
        rec={'world':w.get_path_name(),'items':[]}
        for a in items(w):
            sm=a.get_component_by_class(unreal.StaticMeshComponent);sphere=a.get_component_by_class(unreal.SphereComponent)
            origin,extent,_=unreal.SystemLibrary.get_component_bounds(sm)
            loc=a.get_actor_location();velocity=a.get_velocity()
            # Known representation gap to the collision sphere's lowest point; record settling separately.
            rec['items'].append({'name':a.get_name(),'state':str(a.get_editor_property('sub_alias')),
                'mesh':sm.static_mesh.get_path_name(),'scale':list(sm.get_world_scale().to_tuple()),
                'location':list(loc.to_tuple()),'speed_cm_s':velocity.length(),
                'visual_bottom_z':origin.z-extent.z,'sphere_bottom_z':loc.z-sphere.get_scaled_sphere_radius(),
                'visual_to_sphere_bottom_gap_cm':origin.z-extent.z-loc.z+sphere.get_scaled_sphere_radius(),
                'attached':bool(a.get_attach_parent_actor())})
        report['worlds'].append(rec)
    (root/'docs/asset/evidence/008-meat-pie.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    assert all(len(w['items'])>=3 for w in report['worlds'])
    assert all('/Items/MeatStates/' in a['mesh'] for w in report['worlds'] for a in w['items'])
    print(json.dumps(report))
