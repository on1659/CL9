---
name: spritemake
description: CL9에서 SpriteMake를 호출하여 2D 스프라이트·투명 PNG·아이콘·애니메이션 아틀라스의 제작 배치, 프롬프트, manifest, QA, 프리뷰와 게임 납품을 준비한다. /spritemake 또는 SpriteMake 제작·검수 요청에 사용한다.
---

# SpriteMake

이 스킬의 공통 작업 지침은 [Codex/Claude 공통 SpriteMake 스킬](../../../.agents/skills/spritemake/SKILL.md)에 있다. 해당 파일을 읽고 이번 요청에 적용한다. 현재 절대 경로는 `C:\Work\CL9\.agents\skills\spritemake\SKILL.md`이다.

`/spritemake` 뒤의 요청문을 사용자 작업으로 받아들인다. 인자가 없으면 사용 예시를 안내한다: `/spritemake CL9 음식 아이콘 세팅해줘`.

이미지 생성 도구가 제공되지 않은 세션에서는 공통 지침에 따라 프롬프트·배치를 준비하고 실제 생성이 남았음을 알린다. API 키나 이미지 API 클라이언트를 추가하지 않는다.
