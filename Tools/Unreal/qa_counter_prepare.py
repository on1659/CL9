# Reuse the already verified four-fixture setup with this facility's exact class/mesh.
code=open('D:/Work/unreal/CL9/Tools/Unreal/qa_return_jar_prepare.py',encoding='utf-8').read()
replacements={
 'QA019_':'QA021_', '019-jar-fixtures.json':'021-counter-fixtures.json',
 '/Game/Items/BP_CL9Jar.BP_CL9Jar_C':'/Game/Items/BP_CL9Counter.BP_CL9Counter_C',
 '/Game/Items/ReturnJar/SM_ReturnJar_A':'/Game/Items/DeliveryCounter/SM_DeliveryCounter_A',
 "track(jar,'Jar')":"track(jar,'Counter')",'19998.8,0,1140':'20000,0,1150',
 'Prepared one actual Jar BP':'Prepared one actual Counter BP'}
for before,after in replacements.items():
    assert before in code,before;code=code.replace(before,after)
exec(compile(code,'qa_counter_prepare.py','exec'))
