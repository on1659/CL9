# 소 탈 몬스터 — 기성 모션·의상 도구 조사

조사일: 2026-10-16 (Asia/Seoul)

## 목적과 현재 상태

소 탈 몬스터의 이동·기상·줍기 동작에 쓸 기성 애니메이션과, 벗은 옷이 중력으로 떨어지는 표현을 도울 도구를 조사했다. Fab 후보 조사를 이어가면서 Mixamo·Rokoko·ActorCore/Reallusion도 확인했다.

이 문서는 조사와 이후 확인 기록이다. 유료 구매·설치는 하지 않았고, 이후 사용자 지시로 진행한 무료 모션 시험·다운로드는 아래에 연결한다. 현재 승인본과 검증된 기존 작업은 유지한다. 제작 상태의 정본은 [기존 제작 카드](../monster/disguise-trio-next-production.md)다.

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

### 무료 자료 확보·기상 시험 및 10만 원 예산 판단

후속 사용자 요청에 따라 [Getting Up 5초·1배속 영상](../../../ArtSource/monster/storage_owner_disguise/mixamo_getup_probe_v02/review/cow_getting_up_5s_1x.mp4)을 제작했다. 기존 체형·리그와 기존 836프레임의 손 집기·옷 동작은 보존했다. 실제 QA·재개 상태는 기존 제작 카드의 `cow-mixamo-getup-v02-current` 절을 따른다.

관련 무료 자료는 [라이브러리 manifest](../../../ArtSource/monster/storage_owner_disguise/mixamo_free_library_20261016/manifest.json)에 파일·해시를 기록했다. 개별 모션 17개와 무료 Creature NPC/Scary Zombie/Not So Scary Zombie 3개 팩의 48개 모션을 합쳐 FBX 65개다. 팩 간 중복이 있을 수 있으며 서비스 전체 다운로드나 전부 적용 완료를 뜻하지 않는다.

유료 모션 예산 100,000원 내 우선 후보는 Crawling Ghoul의 **개인용 ₩29,480(VAT 포함)**이다. 느린/빠른 네발 8방향 이동 묶음을 제공하지만 우리 리그·UE 5.8 적용은 미검증이다. 기업용은 ₩147,510로 예산을 넘으므로 해당 자격을 확인하지 않고 개인용 가격으로 구매하지 않는다. 이번 무료 기상 시험은 성공했으며, 현재 Dragon IK·Werewolf를 추가 구매할 필요는 확인되지 않았다. 결제는 실행하지 않았다.

### Fab 가격·호환성 추가 확인 (2026-10-16)

실제 Fab 브라우저의 라이선스 메뉴에서 확인한 원화 가격이다. 아래 금액은 해당 메뉴에 VAT 포함으로 표시됐다. 구매는 하지 않았다. 판매가와 할인은 이후 바뀔 수 있다.

| 후보 | Personal/개인용 | Professional/기업용 | 페이지에 표시된 Unreal 버전 |
|---|---:|---:|---|
| Crawling Ghoul Anims | ₩29,480 | ₩147,510 | 5.2–5.7; 5.8 직접 검증 전 |
| Dragon IK | **₩44,250 (50% 할인)** | **₩59,000 (50% 할인)** | 4.21–4.27 및 5.0–5.8 |
| Auto Drape | ₩44,240 | ₩147,510 | 설명에 5.6·5.7·5.8 명시 |
| Werewolf Biped & Quadruped | ₩50,140 | ₩147,510 | 5.7; 5.8 직접 검증 전 |
| [Zombie Animations — RamsterZ](https://www.fab.com/listings/2dc6e0e8-434c-4946-ad44-5e1b671fc629) | ₩38,340 | ₩154,880 | 4.24–4.27 및 5.0–5.7; 5.8 직접 검증 전 |

Dragon IK 메뉴에는 할인 전 ₩88,501 / ₩118,006 및 할인 종료 `2026-10-18 06:00`이 표시됐다. 종료 시각은 해당 브라우저의 표시값이다. 라이선스 종류는 매출·펀딩 조건에 따라 선택해야 하며, 작은 금액을 임의로 적용하지 않는다.

**기상 모션 판단:** Werewolf에는 Stand Up/Stand Down과 두발↔네발 전환이 명시돼 있다. Crawling Ghoul의 땅속 출입은 옷에서 빠져나오는 동작과 다르다. Zombie Animations 설명에는 기기·이동·공격 등이 있지만, 원하는 특정 기상 클립의 포함은 확인되지 않았다. [좀비 팩 공식 시연](https://youtu.be/iqrKrruGlz0). 기상 하나만 필요하다면 무료 Mixamo Getting Up과 기존 동작부터 비교하고, 그것만을 위해 모델 포함 팩이나 대형 좀비 팩을 먼저 구매할 이유는 약하다.

**IK 판단:** 체형이 다른 모션을 옮길 때 손발 접지와 기존 옷 집기 위치를 유지하는 IK 보정은 필요하다. 하지만 유료 Dragon IK가 필수인 것은 아니다. 현재 평지 비교에는 기존 Blender 접지 보정을 우선 사용한다. Unreal의 [기본 IK Rig](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-in-unreal-engine)와 [Control Rig Full-Body IK](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-full-body-ik-in-unreal-engine)도 대안이다. 불규칙 지형에서 여러 사족 몬스터의 발·척추·시선을 계속 보정해야 할 때 Dragon IK의 도입 비용과 절약 시간을 별도로 비교한다. 옷의 낙하·주름은 이 IK 구매로 해결되는 항목이 아니다.

- **확인 완료:** 위에 적은 Mixamo 동작의 목록 이름·설명, 공식 상품 설명과 시연 링크. Auto Drape 상품 설명의 UE 5.8 지원 명시.
- **미검증:** 우리 본체·옷에 모션을 실제로 옮긴 결과, 손발 접지와 관통, 같은 옷의 재착용, 성능과 최종 패키징. 다른 Unreal 후보의 UE 5.8 지원은 이 문서에서 확정하지 않는다.
- **외형 유지:** 구매 모델로 기존 승인 본체를 대체한다는 결정은 없다. 기존 체형과 손 집기를 유지하는 것이 비교 기준이다.
- **제작 범위 유지:** 이 조사는 기존 원본·검증 자료를 교체하거나, `Content/Peb`의 구매 팩을 수정할 근거가 아니다.

## 후속 시험을 한다면

조사 당시 제안이며, 이후 사용자 지시로 아래 1–2번의 짧은 비교 시험을 실행했다. 현재 실제 결과는 [기존 제작 카드의 Mixamo 시험](../monster/disguise-trio-next-production.md#mixamo-무료-모션-비교-시험--영상-제작-완료사용자-검토-대기-2026-10-16)을 따른다. [7초·1배속 비교 영상](../../../ArtSource/monster/storage_owner_disguise/mixamo_motion_probe_v01/review/cow_mixamo_vs_v20_walk_pickup_7s_1x.mp4)은 기존 v20과 무료 모션 보강 시험을 나란히 보여준다. 유료 상품 구매·설치는 하지 않았다.

1. Mixamo `Running Crawl` 한 주기를 기존 괴물에 옮겨 네 발의 접지·무게 이동·미끄러짐을 비교한다.
2. `Lifting Object`에서 몸을 낮추고 양손으로 드는 구간만 비교한다. 기존 옷 집기 위치는 유지한다.
3. 옷은 손에 붙어 당겨지는 구간과 놓은 뒤 떨어지는 구간을 연결해 검수한다. 기존 바닥의 같은 옷을 재사용한다.
4. 먼저 해당 구간만 1배속 영상으로 보고 채택 여부를 판단한다. 미검증 도구 도입을 위해 전체 영상을 다시 촬영하지 않는다.

기존 제작 결과와 상세 진행 상태는 [소 탈 몬스터 제작 카드](../monster/disguise-trio-next-production.md)를 따른다.

## 탈착에 직접 도움이 되는 제작 보조 도구 — 2026-10-16 추가 조사

이 절은 새 구매를 실행한 기록이 아니다. 사용자가 이후 Crawling Ghoul을 구매하고 5.7용을 다운로드했다고 알렸으며, `Content/CrawlingGhoulAnims`에 파일이 들어온 것을 확인했다. 앞 절의 미구매 표현은 조사 당시 기록이다. UE 5.8 적용은 별도 검증 전이다.

| 후보 | 확인 가격 | 우리 옷 탈착에 보탤 수 있는 부분 | 공식 실제 시연과 한계 |
|---|---:|---|---|
| [Simply Cloth Studio 2.0](https://superhivemarket.com/products/simply-cloth) | 1인 Studio $36; 5좌석 $270. 세금·카드 환산 별도. 상품의 과거 FAQ와 현재 팀 옵션 표기가 섞여 있으므로 팀 라이선스는 별도 확인 필요 | Blender 천 고정점, 충돌/자기 충돌, 재질 프리셋과 베이크 설정. 손에 잡힌 지점을 제어하고 자유 영역의 처짐을 제작할 때 설정 작업을 줄이는 후보 | [제작자 51분 사용법](https://www.youtube.com/watch?v=rSP8X2DfGX4), [5분 지점](https://www.youtube.com/watch?v=rSP8X2DfGX4&t=300s). 실제 천 시뮬레이션/봉제 화면 표본 확인. 완성된 탈착·재착용 모션 제공을 확인한 것은 아니며, Unreal 런타임 플러그인이 아니다. 상품 표시 Blender 4.0–5.2. 제작자 설명상 시뮬레이션 자체 속도보다 설정·반복 작업을 줄이는 도구다 |
| [Auto Drape](https://www.fab.com/listings/ca167eb0-349f-41d5-b713-290c67836879) | 개인용 ₩44,240 VAT 포함; 기업용 ₩147,510 예산 초과. Fab 메뉴 재확인 | 손을 놓은 뒤 낙하·접힘·바닥 정착 형태를 저작/베이크하는 후보 | [제작자 6분48초 사용법](https://www.youtube.com/watch?v=E0QyVQhnhdA), [전체 워크스루](https://www.youtube.com/watch?v=OmkeF_x7xJA). UE 5.6–5.8 지원 명시. 자기 옷 Dropped Garment는 베타. 손 잡기·몸 통과·주워 재착용의 완성 시스템은 아님 |

검색 범위에서 10만 원 이내에 우리 사족 괴물의 같은 옷 전체 탈착과 재착용을 그대로 해결하는 판매 에셋·시연은 확인하지 못했다. 보조 도구가 있다는 사실을 완성 동작 구매 가능으로 해석하지 않는다. 기존 Blender 천/IK/형태 키 제어와 겹치는 기능도 있으므로 자동 해결을 기대한 추가 구매는 권하지 않는다. 구매 없이 공식 시연을 브라우저에 제공했다.

### 탈의 제작을 실제로 시연한 우선 후보: Marvelous Designer 개인용 월간

사용자가 손가락은 잡는 시늉 수준이면 충분하며, 탈의→낙하→같은 옷 줍기→재착용에 직접 도움이 되는 도구를 다시 요청했다. 이에 일반 낙하 도구와 구분해 조사했다.

- **가격:** [공식 가격 페이지](https://www.marvelousdesigner.com/pricing)에서 개인용 월간 전환 후 **39 USD / Month, Billed monthly**를 확인했다. 개인용 한 달 기준 약 5~6만 원대 예상이며 세금·카드 환율에 따라 달라진다. 영구 라이선스가 아니고 자동 갱신 구독이다. 연간 환산 월 $23.3 표시는 연간 결제이므로 10만 원 예산 구매안으로 제시하지 않는다. 조직용 가격은 다르다.
- **실제 탈의 시연:** RenderRides의 [Tutorial: Animate a Character Taking Off Clothes](https://www.youtube.com/watch?v=YJ8VZqw2aAk), 11분48초. 제작자가 Marvelous Designer 2025와 Blender로 한 샷에서 티셔츠를 벗는 전체 제작 과정을 설명한다. 제작자 [튜토리얼 목록](https://www.renderrides.com/tutorials)에도 연결돼 있다. MD 후원 영상이라는 설명이 있으며, 사람의 티셔츠 탈의 사례를 우리 소 옷 전체 주기 성공 증거로 확대하지 않는다.
- **제어 근거:** [공식 Attach/Detach Pin 문서](https://support.marvelousdesigner.com/hc/en-us/articles/47358452805913-Attach-Pin-to-Detach-Pin-from-Avatar)는 옷의 부분을 아바타 가까운 표면에 고정/해제하는 기능을 설명한다. [공식 FBX 문서](https://www.marvelousdesigner.com/manuals/articles/47358232885017)는 움직이는 아바타 임포트와 옷 애니메이션 캐시 경로를 설명한다.
- **우리 작업의 이득:** 본체·손의 경로는 기존 Blender 제어로 유지하고, 옷을 잡아 빼는 접촉 및 자유 영역의 충돌·낙하·접힘을 별도 천 제작 도구로 저작/시뮬레이션할 후보다. 같은 가죽의 재착용은 별도 손/몸 경로와 접촉 타이밍을 제작해야 한다. 머리·뿔 같은 단단한 부위와 천 몸통도 구분해야 한다.
- **한계:** 완성 탈착 모션 팩이나 Unreal 실시간 탈착 플러그인이 아니다. 기존 소 옷 메시 적합성·전체 재착용·게임용 변환·소비 리그 연결은 시험 전이다. 이 구매로 완성이 보장된다고 설명하지 않는다. 구매·설치·구독 시작은 하지 않았다.
- **체험:** [공식 FAQ](https://support.marvelousdesigner.com/hc/en-us/articles/47358247243289-How-can-I-get-a-trial-version-of-Marvelous-Designer)는 14일 개인용 체험과 종료 후 자동 월간 결제를 안내한다. 체험이라고 무결제 영구 도구로 취급하지 않는다.

### 구독 제외 조건으로 재검색 — 2026-10-16

사용자가 월 구독을 선호하지 않아 무료/일회성 구매 도구로 범위를 좁혔다. 이번은 조사만이며 설치·구매·새 시뮬레이션을 실행하지 않았다.

| 후보 | 비용/구독 여부 | 확인한 실제 제어·제약 |
|---|---|---|
| [Cloth Lab v0.8.1](https://slohero.com/tools/blender/en/) | 무료, GPL-3.0-or-later | 제작자 페이지가 핀·가중치·Hook·키프레임을 통한 잡기/이동/모으기/해제를 명시한다. Blender 기본 천 솔버 기반. Windows Blender 5.2.2 검증은 제작자 주장이고 우리 환경 적용 검증은 전이다. [공개 천 시연](https://slohero.com/tools/blender/assets/cobalt-reel.mp4)은 천을 모으고 흐르게 하는 사례이며 전체 탈착·재착용 영상이 아니다. 무료 시험 우선 후보 |
| [Simply Cloth Studio](https://superhivemarket.com/products/simply-cloth) | 1인 $36 일회성; 팀 옵션 $270은 예산 초과 | 천 고정점·충돌·베이크 설정 보조. [Superhive 지원 정책](https://support.superhivemarket.com/article/340-support-periods-overview)은 12개월 지원이 구독이 아니며 자동 갱신/정기 청구가 없고 받은 파일/버전은 계속 사용할 수 있다고 설명한다. Blender 버전업 때의 미래 호환성과 무제한 미래 업데이트를 보장한다는 뜻은 아니다 |
| [Grab-it](https://www.blendahaven.com/product/grab-it) | $25 표시 | 물체/뼈에 붙이기·놓기 타이밍 제작 편의 도구. 천 변형·몸 통과·낙하 물리를 자체 해결하지 않으므로 이번 옷 문제의 우선 구매로 권하지 않는다. Blender 5.2 별도 실검증 전 |

Modeling Cloth의 $35 Gumroad 페이지는 Blender 2.8/2.79 안내가 남아 있어 현재 5.2 호환성 확인 없이 구매 후보로 추천하지 않는다. 현재 제안은 무료 Cloth Lab의 고정/해제 제어를 짧게 시험하는 것이며, 설치·사용 승인을 받았다는 뜻은 아니다. 어떤 후보도 같은 소 옷의 전체 탈착/재착용을 구매 즉시 완성한다고 설명하지 않는다.
