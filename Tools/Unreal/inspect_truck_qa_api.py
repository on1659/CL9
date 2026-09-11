import unreal
print('Module game instance API:',[n for n in dir(unreal) if 'game_instance_subsystem' in n.lower()])
print('Module subsystem libraries:',[n for n in dir(unreal) if 'subsystem' in n.lower() and ('library' in n.lower() or n.startswith('get_'))])
print('GameInstance methods:',[n for n in dir(unreal.GameInstance) if 'subsystem' in n.lower()])
print('Object access:',[n for n in dir(unreal) if n in ['ObjectIterator','ObjectBaseIterator','get_objects_of_class','find_object','CL9UIManager','CL9ShopWidget']])
if hasattr(unreal,'ObjectIterator'):
 for obj in unreal.ObjectIterator(unreal.CL9UIManager):
  print('UI manager',obj.get_path_name(),obj.get_outer().get_path_name())
