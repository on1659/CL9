"""Real cart load/unload/interact RPCs; controlled pawn relocation probes towing tick."""
import unreal,json,datetime,math
from pathlib import Path
root=Path(unreal.Paths.project_dir());worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
host=next(w for w in worlds if 'UEDPIE_0_' in w.get_path_name())
pawn=unreal.GameplayStatics.get_player_pawn(host,0);pc=unreal.GameplayStatics.get_player_controller(host,0);ps=pawn.player_state
assert str(ps.hand_item.item_alias)=='None'
actors=unreal.GameplayStatics.get_all_actors_of_class(host,unreal.Actor)
cart=next(a for a in actors if a.get_class().get_name()=='BP_CL9Cart_C')
egg=next(a for a in actors if a.get_class().get_name()=='BP_CL9Item_C' and str(a.get_editor_property('alias'))=='egg' and str(a.get_editor_property('sub_alias'))=='None')
meta=json.loads((root/'ArtSource/item/cart_a/export/manifest.json').read_text())['assets']['cart']
out=root/'docs/asset/evidence/011-cart-use.json'
if out.exists():
    old=json.loads(out.read_text());assert old['status']!='running','Inspect live callback before retrying'
    out.with_name('011-cart-use-attempt-'+datetime.datetime.now(datetime.timezone.utc).strftime('%H%M%S')+'.json').write_text(out.read_text(),encoding='utf-8')
original_cart_position=list(cart.get_actor_location().to_tuple())
radius=cart.get_component_by_class(unreal.SphereComponent).get_scaled_sphere_radius()
if cart.get_actor_location().z<radius+1:
    loc=cart.get_actor_location();cart.set_actor_location(unreal.Vector(loc.x,loc.y,radius+2),False,True)
state={'status':'running','utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'world_count':len(worlds),
       'engine':unreal.SystemLibrary.get_engine_version(),'level':host.get_path_name(),'elapsed':0,'step':0,'events':[],
       'scope':'Real pickup/load/unload/interact RPC; pawn teleport setup tests towing response, not walking inputs/2-player.',
       'setup_cart_original_position':original_cart_position,'setup_cart_corrected_position':list(cart.get_actor_location().to_tuple())}
def save():out.write_text(json.dumps(state,indent=2),encoding='utf-8')
def xy_distance(a,b):return math.hypot(a[0]-b[0],a[1]-b[1])
def aim(target):
    loc=target.get_actor_location();half=pawn.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_half_height()
    pawn.set_actor_location(unreal.Vector(loc.x-45,loc.y,half+2),False,True)
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),loc))
def capture(label):
    sm=cart.get_component_by_class(unreal.StaticMeshComponent);es=egg.get_component_by_class(unreal.StaticMeshComponent)
    origin,extent,_=unreal.SystemLibrary.get_component_bounds(sm);eo,ee,_=unreal.SystemLibrary.get_component_bounds(es)
    loc=cart.get_actor_location();parent=egg.get_attach_parent_actor();rot=cart.get_actor_rotation()
    assert abs(rot.pitch)<.01 and abs(rot.roll)<.01
    scale=sm.get_world_scale().z;deck_z=loc.z+meta['anchors_cm']['CargoFloor'][2]*scale
    e={'event':label,'hand':str(ps.hand_item.item_alias),'cart_location':list(loc.to_tuple()),
       'pawn_location':list(pawn.get_actor_location().to_tuple()),'mesh':sm.static_mesh.get_path_name(),
       'scale':list(sm.get_world_scale().to_tuple()),'cart_visual_bottom_z':origin.z-extent.z,
       'cart_sphere_bottom_z':loc.z-cart.get_component_by_class(unreal.SphereComponent).get_scaled_sphere_radius(),
       'egg_parent':parent.get_name() if parent else None,'egg_bottom_z':eo.z-ee.z,'deck_z':deck_z,
       'egg_to_deck_gap_cm':eo.z-ee.z-deck_z,
       'egg_relative_location':list(egg.get_component_by_class(unreal.SphereComponent).get_editor_property('relative_location').to_tuple())}
    state['events'].append(e);save();assert e['mesh'].endswith('/SM_Cart_A.SM_Cart_A');return e
aim(egg);save()
def tick(delta):
    try:
        assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
        state['elapsed']+=delta
        if state['elapsed']<(state['step']+1)*1.2:return
        step=state['step']
        if step==0:pawn.call_method('Server_PickPlace')
        elif step==1:
            e=capture('egg_held');assert e['hand']=='egg' and e['egg_parent']==pawn.get_name();aim(cart)
        elif step==2:pawn.call_method('Server_PickPlace')
        elif step==3:
            e=capture('loaded');assert e['hand']=='None' and e['egg_parent']==cart.get_name();aim(egg)
        elif step==4:pawn.call_method('Server_PickPlace')
        elif step==5:
            e=capture('unloaded');assert e['hand']=='egg' and e['egg_parent']==pawn.get_name()
            pc.set_control_rotation(unreal.Rotator(pitch=35,yaw=pc.get_control_rotation().yaw+180,roll=0))
        elif step==6:pawn.call_method('Server_PickPlace')
        elif step==7:
            assert str(ps.hand_item.item_alias)=='None';aim(cart);state['before_pull']=list(cart.get_actor_location().to_tuple())
        elif step==8:pawn.call_method('Server_PickPlace')
        elif step==9:
            e=capture('tow_connected');assert xy_distance(e['cart_location'],state['before_pull'])<.1,'Cart jumped when connecting'
            p=pawn.get_actor_location();pawn.set_actor_location(p+unreal.Vector(0,300,0),False,True)
        elif step==10:
            e=capture('tow_followed')
            assert xy_distance(e['cart_location'],state['before_pull'])>50,'Cart did not follow'
            hitch=pawn.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_radius()+80
            assert abs(xy_distance(e['cart_location'],e['pawn_location'])-hitch)<2,'Unexpected hitch distance'
            pc.set_control_rotation(unreal.Rotator(pitch=60,yaw=0,roll=0))
        elif step==11:pawn.call_method('Server_PickPlace')
        elif step==12:
            state['released_at']=capture('tow_released')['cart_location']
            pawn.set_actor_location(pawn.get_actor_location()+unreal.Vector(0,200,0),False,True)
        elif step==13:
            e=capture('released_stays');assert xy_distance(e['cart_location'],state['released_at'])<1,'Cart still follows after release'
            state['status']='passed_local_load_unload_tow_response';save();unreal.unregister_slate_post_tick_callback(handle);return
        state['step']+=1;save()
    except Exception as error:
        state['status']='failed';state['error']=str(error);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick);print('Cart use callback registered: '+str(out))
