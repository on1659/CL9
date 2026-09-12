# 납품 창구

소비처 `ACL9Counter`/`ACL9Fixture`, `/Game/Items/BP_CL9Counter`.
[실제 조회](../evidence/021-counter-consumer.json): Lvl_Flat의100cm 기본 큐브/중심Z50/배율1.
새 모델100×100×100cm/바닥 중앙 피벗/+X 정면. 기존 인스턴스는 교체 때 바닥Z0으로 보정한다.

조리대와 이어지는 법랑·짙은 금속 재질에 푸른 전면 판, 배송 상자와 확인 표시를 넣는다.
넓은 평평한 접수 상판과 측면 서류판으로 열원/녹색 반납 항아리와 구별한다.
UV0/상수 재질3개/상판·몸통·다리 단순 충돌. 외부 글꼴/그림 의존성 없는 자체 모델.
원본 `ArtSource/map/delivery_counter/`. 고정 시설이며 현재 가동부가 없어 리그/클립 N/A.

실제 E 동작은 손/수레의 물건을 재고로 받아 없애고 주문을 처리하거나, 빈손일 때 남은 재고를 판다.
전시된 물건이 쌓이거나 서랍/벨이 움직이는 기능을 새로 추가하지 않는다.
실제 Counter BP 임시 인스턴스로 납품/재고/판매를 검수하고 원본 BP·레벨은 보존한다.

납품 실측100×100×100cm/1,552 tris/854 vertices/UV0/3상수 재질/UCX6.
원본 `delivery_counter.blend`, `export/SM_DeliveryCounter_A.fbx`, `preview/` 중립·야간·손전등3장.
3조명과 Native UE 에셋 이미지 확인. UE `Items/DeliveryCounter`, 치수 오차<0.00002cm/LOD3.
LOD 전환·성능/영구 연결/기존 인스턴스 바닥 보정/2인 검수는 미완료다.

[021](../iterations/021.md)의 실제 Counter BP1월드 PIE에서 납품/액터 제거/빈손 판매/수입 증가/
중복 수입 없음, 몸통 차단/상판 구체 지지를 확인했다. 내부 재고 배열은 protected라 직접 비교하지 않았다.
주문 완납/수레/영구 연결/2인/LOD·성능은 미완료다.
