code=open('D:/Work/unreal/CL9/Tools/Unreal/inspect_return_jar.py',encoding='utf-8').read()
code=code.replace("if 'Jar' not in a.get_class().get_name()","if 'Counter' not in a.get_class().get_name()")
exec(compile(code.replace('019-jar-consumer.json','021-counter-consumer.json'),'inspect_counter.py','exec'))
