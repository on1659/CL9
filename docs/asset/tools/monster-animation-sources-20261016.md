# 소 탈 몬스터 — 기성 모션·의상 도구 조사

조사일: 2026-10-16 (Asia/Seoul)

## 목적과 현재 상태

소 탈 몬스터의 이동·기상·줍기 동작에 쓸 기성 애니메이션과, 벗은 옷이 중력으로 떨어지는 표현을 도울 도구를 조사했다. Fab 후보 조사를 이어가면서 Mixamo·Rokoko·ActorCore/Reallusion도 확인했다.

이 문서는 조사 기록이다. 구매·설치·새 리깅·모션 교체·추가 촬영을 실행했다는 뜻이 아니다. 현재 승인본, 실행 중 작업, 통과한 검사는 유지한다. 제작 상태의 정본은 [기존 제작 카드](../monster/disguise-trio-next-production.md)다.

## 판단

1. **무료 비교는 Mixamo부터:** `Running Crawl`로 네발 이동, `Lifting Object`로 양손 줍기의 몸 움직임을 비교할 수 있다. 실제 라이브러리 목록에서 해당 이름과 설명을 확인했다.
2. **유료 이동 후보는 Crawling Ghoul 우선 검토:** 네발로 움직이는 사람형 괴물용이며, 긴 팔로 바닥을 짚는 우리 본체에 적용할 후보로 판단한다. 실제 체형 적합성과 수정 비용은 적용 시험 전까지 미확정이다.
3. **옷의 낙하는 별도 검토:** Auto Drape는 바닥으로 떨어지고 접히는 옷을 저작하는 후보지만, 손으로 벗고 같은 옷을 다시 입는 완성 시스템은 아니다.
4. 이번에 확인한 상품 설명에서는 **손으로 옷 잡기 → 어깨에 걸림 → 골반·마지막 뒷발 빠짐 → 손에서 놓아 낙하 → 같은 옷 줍기 → 다시 입기** 전체를 제공하는 패키지를 찾지 못했다. 기존 체형·손 집기와 옷 동작을 유지한 맞춤 수정이 필요하다.

## Mixamo — 확인한 무료 동작

- [기기 검색 목록](https://www.mixamo.com/#/?page=1&query=crawl&type=Motion%2CMotionPack)
- [줍기 검색 목록](https://www.mixamo.com/#/?page=1&query=pick+up&type=Motion%2CMotionPack)
- [Adobe 공식 이용 조건·지원 체형 설명](https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html)
- [Adobe 공식 애니메이션 미리보기·설정 안내](https://helpx.adobe.com/creative-cloud/help/animate-characters-mixamo.html)

| 실제 목록 이름 | 목록에 표시된 동작 | 우리 작업에서의 용도 |
|---|---|---|
| Running Crawl | Running On All Fours | 네발 달리기 비교 우선 후보 |
| Zombie Crawl | Zombie Crawling Forward | 낮게 기는 괴물 이동 비교 |
| Crawling | Crawling Forward On Hands And Knees | 기기 동작의 체중 이동 참고; 무릎 자세는 우리 체형에 맞게 수정 필요 |
| Low Crawl | Super Low Crawl, Or Crawling Up A Wall | 낮은 자세 비교 |
| Getting Up | Getting Up From Back / Getting Up From Stomach | 바닥에서 일어나는 몸의 흐름 참고 |
| Picking Up | Picking Up An Object | 줍기 동작 참고 |
| Picking Up Object | Picking Up An Object With One Hand | 한 손 줍기 참고 |
| Lifting Object | Lifting A Heavy Object With Both Hands | 양손으로 같은 옷을 들어 올리는 몸 움직임 비교 우선 후보 |
| Dropping | Dropping Something And Picking It Up | 놓기·다시 줍기의 연결 참고 |

Mixamo는 Adobe ID로 무료 이용할 수 있고, 공식 FAQ는 캐릭터·애니메이션의 게임 등 상업 프로젝트 사용을 허용한다고 설명한다. 자동 리깅과 애니메이션 라이브러리의 공식 지원 대상은 **사람형 이족 캐릭터**다. 과도한 체형 변형이나 큰 부속물·의상은 자동 리깅에 문제가 될 수 있다.

따라서 우리 괴물에 자동으로 맞는다고 판단하지 않는다. 기존 리그에 모션을 옮긴 뒤 팔·다리 길이, 손발 접지, 숙인 척추와 머리 움직임을 보정해야 한다. 줍기 모션에도 실제 소 옷 메시와의 접촉·변형은 별도로 맞춰야 한다.

## 사이트별 후보와 시연 영상

아래 영상은 공식 상품 또는 제작자 페이지에서 연결된 시연 링크다. 링크 확보와 상품 설명 확인을 실제 영상 전체의 재생 검수나 우리 모델 적용 검증으로 취급하지 않는다.

| 후보 | 확인한 구성·역할 | 적용 판단·제약 | 시연 |
|---|---|---|---|
| [Crawling Ghoul Anims — Fab](https://www.fab.com/listings/9e96b3f7-30ca-4221-a2eb-2bd46146b286) | 개구리처럼 숙인 사람형 네발 괴물. 빠르고 느린 8방향 기기, 대기·공격·회전 등. 업데이트 포함 75개 애니메이션 | 이동용 유료 우선 후보. 홍보 모델과 게임플레이 Blueprint는 미포함. 우리 리그로 옮기는 작업 필요 | [기본 시연](https://www.youtube.com/watch?v=PIqLMUrNa_4), [v1.1 업데이트](https://youtu.be/W-8dqN2P9SE) |
| [Ghost Creature Anims — RamsterZ](https://www.ramsterzanimations.com/store-buy/p/ghost-creature-anims-fbx-only) | 기기·대기·이동·전환·짝 동작을 포함하는 FBX 모션 묶음 | 기기와 자세 전환의 추가 후보. 상대를 잡는 동작을 옷 잡기 완성 동작으로 해석하면 안 됨. 모델·게임 로직 별도 | [v1.1 업데이트](https://youtu.be/9TJKK1LjYjg) |
| [Werewolf Biped & Quadruped — Fab](https://www.fab.com/listings/42afba45-1120-4c61-92b7-76ba8e1e2d30) | 리깅된 늑대인간 모델과 50개 애니메이션. 두발·네발 이동 및 자세 전환, 표정 포함 | 기상·두발↔네발 전환 참고. 우리 본체에 쓸 경우 리그 적합성 확인 필요. 의상 유무 선택은 실제 옷 벗기 동작을 뜻하지 않음 | [상품 시연](https://www.youtube.com/watch?v=_ML39NeaFtQ) |
| [Rokoko 무료 좀비 12종](https://www.rokoko.com/resources/rokoko-mocap-12-free-zombie-animations) | 전신·손가락 모캡, Mixamo 뼈대, FBX, 30fps. 배포 설명상 상업 프로젝트 사용 가능 | 무료 비교 후보. 개별 클립 목록과 별도 시연 영상은 이번 조사에서 확보하지 못함. 배포 페이지에 이름·이메일 입력 및 마케팅 동의 양식이 있음 | 공식 배포 페이지에서 확인; 개별 영상 미확보 |
| [ActorCore](https://actorcore.reallusion.com/) / [Reallusion Zombie Motions](https://www.reallusion.com/contentstore/pack/iclone/3d-animation/zombie-motions) | 구체적으로 확인한 Zombie Motions는 3가지 좀비 스타일, 23개 rlMotion 동작. 걷기·달리기·공격·죽음 등 | 사람형 좀비 움직임 후보. 해당 상품은 rlMotion 형식이므로 직접 FBX 제공 여부 및 필요한 내보내기 경로를 확인해야 함. ActorCore의 모든 상품이 같은 구성이라는 뜻은 아님 | [Zombie Motions 시연](https://www.youtube.com/watch?v=NK9pmVY_Meg) |
| [Auto Drape — Fab](https://www.fab.com/listings/ca167eb0-349f-41d5-b713-290c67836879) | 천을 떨어뜨려 정착시키고 Static Mesh·Skeletal Mesh·GeometryCache로 출력. UE 5.6·5.7·5.8 Windows 지원 명시 | 옷 낙하·바닥 주름 저작 후보. 자기 옷 메시를 사용하는 Dropped Garment는 베타. 전체 탈착이나 런타임 재착용을 자동 해결한다고 설명하지 않음 | [시연](https://www.youtube.com/watch?v=OmkeF_x7xJA), [사용법](https://youtu.be/E0QyVQhnhdA) |
| [Dragon IK — Fab](https://www.fab.com/listings/d3f8d256-d8d9-4d27-91c1-c61e55e984a6?lang=en) | 네발·두발 등의 발 배치, 척추, 조준·시선, 속도 보정 등 | 기존 모션의 접지와 고개 보정 후보. 모션 팩이나 옷 벗기 도구가 아님. 평평한 테스트 바닥을 위해 반드시 구매해야 하는 것은 아님 | [시연](https://www.youtube.com/watch?v=3EBvLRNMsFY), [네발 IK 사용법](https://www.youtube.com/watch?v=Z3Ny0xfutRI) |
| [VICODynamics — Fab](https://www.fab.com/listings/0249ad1a-981b-4f93-b068-22e42fe9f0e0?lang=en) | 천·로프 등의 입자 기반 물리, 충돌·고정점·힘 처리 | 옷 물리 구현의 추가 후보지만 통합 작업이 필요. 완성된 탈착 시스템이 아니며, 이번 요구에는 도입 범위가 커질 수 있음 | [미리보기](https://youtu.be/7Rs3joj0aMU), [기능](https://youtu.be/rXN_St1CwBk), [시연](https://youtu.be/4xt7ydQWlwA) |

## 적용 전에 구분할 것

- **확인 완료:** 위에 적은 Mixamo 동작의 목록 이름·설명, 공식 상품 설명과 시연 링크. Auto Drape 상품 설명의 UE 5.8 지원 명시.
- **미검증:** 우리 본체·옷에 모션을 실제로 옮긴 결과, 손발 접지와 관통, 같은 옷의 재착용, 성능과 최종 패키징. 다른 Unreal 후보의 UE 5.8 지원은 이 문서에서 확정하지 않는다.
- **외형 유지:** 구매 모델로 기존 승인 본체를 대체한다는 결정은 없다. 기존 체형과 손 집기를 유지하는 것이 비교 기준이다.
- **제작 범위 유지:** 이 조사는 기존 원본·검증 자료를 교체하거나, `Content/Peb`의 구매 팩을 수정할 근거가 아니다.

## 후속 시험을 한다면

아직 실행하지 않은 제안이다. 무료 후보의 짧은 구간부터 기존 결과와 비교하고, 필요한 경우에만 유료 도구를 검토한다.

1. Mixamo `Running Crawl` 한 주기를 기존 괴물에 옮겨 네 발의 접지·무게 이동·미끄러짐을 비교한다.
2. `Lifting Object`에서 몸을 낮추고 양손으로 드는 구간만 비교한다. 기존 옷 집기 위치는 유지한다.
3. 옷은 손에 붙어 당겨지는 구간과 놓은 뒤 떨어지는 구간을 연결해 검수한다. 기존 바닥의 같은 옷을 재사용한다.
4. 먼저 해당 구간만 1배속 영상으로 보고 채택 여부를 판단한다. 미검증 도구 도입을 위해 전체 영상을 다시 촬영하지 않는다.

기존 제작 결과와 상세 진행 상태는 [소 탈 몬스터 제작 카드](../monster/disguise-trio-next-production.md)를 따른다.
