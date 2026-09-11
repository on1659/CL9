code=open('D:/Work/unreal/CL9/Tools/Unreal/inspect_return_jar.py',encoding='utf-8').read()
code=code.replace("if 'Jar' not in a.get_class().get_name()","if not any(n in a.get_class().get_name() for n in ['TruckPad','TruckPanel'])")
exec(compile(code.replace('019-jar-consumer.json','022-truck-consumers.json'),'inspect_truck_parts.py','exec'))
