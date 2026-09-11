code=open('D:/Work/unreal/CL9/Tools/Unreal/inspect_return_jar.py',encoding='utf-8').read()
code=code.replace("if 'Jar' not in a.get_class().get_name()","if 'Kitchen' not in a.get_class().get_name()")
code=code.replace('019-jar-consumer.json','020-kitchen-consumer.json')
exec(compile(code,'inspect_kitchen.py','exec'))
