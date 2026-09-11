"""Seed the production ledger from captured live rows. Never overwrite an existing ledger."""
import json
from pathlib import Path

root = Path(__file__).resolve().parents[2]
target = root/'docs/asset/catalog.md'
if target.exists():
    raise RuntimeError('Catalog already exists; update reviewed rows, do not reset progress')
source = root/'Saved/asset-live-inventory.json'
data = json.loads(source.read_text(encoding='utf-8'))
evidence = root/'docs/asset/evidence'
evidence.mkdir(exist_ok=True)
(evidence/'initial-live-definitions.json').write_text(source.read_text(encoding='utf-8'),encoding='utf-8')
entries=[]
def add(id,category,alias,source,clips,model='미제작',path='—',next='제작 카드 → Blender 제작',rig='N/A: 정적 메시'):
    entries.append([id,category,alias,source,'신규',model,rig,clips,'미완료','미검수','미연결','미실행',path,'없음',next])
for row in data['Item']:
    v=row['values']; alias=v['alias']; sub=v['subAlias']; id=alias+('_'+sub if sub!='None' else '_raw' if alias in ['egg','meat'] else '')
    add(id,'item',alias+'/'+sub,row['path'],'N/A: 상태별 Static Mesh 교체; 운반 동작은 character',
        '샘플 있음' if id=='egg_raw' else '미제작',
        'ArtSource/map/henhouse_sample_01/' if id=='egg_raw' else '—')
for alias in ['pot','standing_lamp']:
    add(alias,'item',alias+'/None','docs/plan/data/equipment.md (실제 행 아직 없음)','N/A: 정적 소품/토글은 재질',next='외형 제작; 게임 기능 연결 미구현 여부 분리')
for row in data['Monster']:
    alias=row['values']['alias']
    clips='Idle, Walk, Run, WakeTelegraph, HitReact, Death'
    clips+=', Sleep, AlertWindup, Honk' if alias=='goose' else ', Catch'
    add(alias+'_a','monster',alias+'/None',row['path'],clips+' — 전부 미제작',rig='미제작',next='현재 종 리그/AnimBP 계약 조사 → 신규 모델·클립')
for alias in ['farmer_a','farmer_b']:
    for part in ['body','arms']:
        add(alias+'_'+part,'character',alias+'/None',
            data['Character'][0]['path'] if alias=='farmer_a' else 'docs/plan/data/character.md: 시작 2종',
            ('Idle, Walk, Run, Jump, Carry, Use, HitReact, Knockdown, GetUp, Death, Exhausted' if part=='body' else
             'Idle, Walk, Run, Jump, Carry, Pickup, Drop, Swap, Throw, Use, HitReact, Knockdown')+' — 전부 미검수',
            rig='기존 Manny/FPP 계약 유지, 검수 전',next='기존 리그 추출/호환 확인 → 작업복 외형·4슬롯 색·운반 동작')
map_parts=[
('henhouse_wall_500_a','wall/henhouse','wall'),('henhouse_wall_half','wallHalf/henhouse',None),
('henhouse_fence','fence/henhouse',None),('henhouse_door','door/henhouse',None),
('henhouse_floor','box/None: Floor용 별도 연결 검토','floor'),
('henhouse_ceiling','box/None: Ceiling용 별도 연결 검토',None),
('henhouse_stair','box/None: 계단 디딤판',None),
('henhouse_nest','둥지 배치','nest'),('henhouse_crate','적재 상자 소품','crate'),
('henhouse_feed_sack','사료실 소품','sack'),('henhouse_work_lamp','시설등','lamp'),
('henhouse_trough','사료통','trough'),('henhouse_beam','보/기둥',None),
('henhouse_sign','방/출구 표식',None),('henhouse_ground','마당/기반 바닥',None),
('henhouse_workbench','작업실/사무실 가구',None)]
for id,alias,sample in map_parts:
    add(id,'map',alias,'DA_PieceTable + DA_ModuleTable + map/henhouse_sample_01.md',
        'N/A: 정적 조립; 문 실제 열림 기능 소비처 확인' if 'door' in id else 'N/A: 정적 환경',
        '샘플 있음' if sample else '미제작', 'ArtSource/map/henhouse_sample_01/' if sample else '—',
        next='샘플 UE 왕복 → 표면/충돌/LOD → 실제 모듈 배치' if sample else '제작 카드 → 치수/소비처 확인 → 제작')
for id,usage in [('food_truck','원정 거점 외형'),('truck_pad','BP_CL9TruckPad'),('truck_panel','BP_CL9TruckPanel'),
                 ('kitchen','BP_CL9Kitchen'),('delivery_counter','BP_CL9Counter'),('return_jar','BP_CL9Jar')]:
    add(id,'map',usage,'Lvl_Flat + docs/systems/cooking.md/raiding.md + Source/CL9/World/',
        'N/A: 고정 시설; 실제 가동부 필요 여부 제작 카드에서 확인')
for row in data['Module']:
    v=row['values']
    add('module_'+v['alias'],'map assembly',v['alias']+'/'+v['subAlias'],row['path'],
        'N/A: 조립/동선 검수 단위, 독립 리그 없음',model='아트 미적용',
        next='완성 부품 배치/역할 구분 → 여러 시드·통로·층·운반 PIE')
header=['ID','분류','Alias/SubAlias·용도','등장 근거','방식','모델','리그','필수 클립/상태','재질/텍스처','충돌/LOD','UE 연결','QA','원본 경로','차단','다음 행동']
intro='''# 전체 에셋 제작 목록

2026-09-11 최초 실측 목록. **완료 0건**. 현재 행과 명시된 기획의 합집합이며, 새로 발견한 실제 자산은 보충한다.
원자료: [UE5.8.1 MCP 실제 정의](evidence/initial-live-definitions.json).
실제 데이터: Item 12행, Monster 5행, Character 1행, Piece 5행, Module 17행, Site 2행, Equipment 1행.
farmer_b, pot, standing_lamp는 명시된 기획 대상이며 런타임 행이 아직 없다. pig/sheep는 실제 테스트 레벨에 있으므로 포함.
벌집 등 미정 미래 스테이지는 현재 필수가 아니다. 기획 밖 신규 종/아이템을 추가하지 않는다.

이 목록은 **납품/조립 검수 단위**다. 같은 부품을 쓰는 17개 모듈을 17개의 별도 메시로 만들 필요는 없다.
wallHalf는 현재 데이터에 있으나 미사용 후보; 사용처 조사 후 근거를 기록해야 제외할 수 있다.
구매 팩·엔진 기본 도형은 교체 전 비교 기준이며, 보유만으로 신규 제작 완료 처리하지 않는다.
모든 행의 재질·충돌·LOD·UE·QA는 개별 증거로 갱신한다. 리그/클립 N/A는 정적인 소품에 한정한다.

캐릭터 4슬롯 색, 상태별 클립은 각 행의 하위 요구다. 같은 클립 공유는 호환 검수 후 기록한다.
아이템 egg는 raw/cooked/burnt/cracked/broken 5상태, meat는 3상태 모두 별도 납품한다.
아이콘/오디오·미정 게임 기능은 이 모델/애니메이션 goal의 별도 신규 기능으로 발명하지 않는다.

'''
table='| '+' | '.join(header)+' |\n|'+ '|'.join(['---']*len(header))+'|\n'
table+='\n'.join('| '+' | '.join(str(c).replace('|','/') for c in row)+' |' for row in entries)+'\n'
target.write_text(intro+f'필수 작업 단위: **{len(entries)}개** (추가 누락 감사 전 기준 목록).\n\n'+table,encoding='utf-8')
print('Created catalog:',len(entries))
