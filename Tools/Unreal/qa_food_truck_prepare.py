PREFIX='QA023_';FIXTURE_EVIDENCE='023-food-truck-fixtures.json';MESH_PATH='/Engine/BasicShapes/Cube'
MESH_PATHS={'Body':'/Game/Maps/FoodTruck/Body/SM_FoodTruck_A','Steps':'/Game/Maps/FoodTruck/Steps/SM_FoodTruck_Steps_A','Kitchen':'/Game/Items/Kitchen/SM_Kitchen_A'}
FIXTURE_SCALES={'Floor':(12,10,.3)};FIXTURE_COLLISION_PROFILES={}
FIXTURES=[('Floor',(20000,0,985),0),('Body',(20000,0,1000),0),('Steps',(19643,78.25,1000),0),('Kitchen',(19953,-33.75,1064),90)]
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/qa_fence_prepare.py',encoding='utf-8').read(),'qa_fence_prepare.py','exec'))
