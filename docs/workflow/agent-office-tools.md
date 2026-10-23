# 에이전트 활동을 사무실처럼 보여 주는 도구 조사

조사일: 2026-10-18. 스타 수와 최근 업데이트는 이날 GitHub API로, 설치 수는 VS Code 마켓으로 직접 조회했다.
설치하거나 실행해 본 도구는 없다. 기능과 지원 플랫폼은 각 저장소 README에 적힌 내용을 옮긴 것이다.

목적: 텍스트 보고만으로는 에이전트(세션·서브에이전트)가 무엇을 하는지 알아보기 어렵다. 그래서 실제 에이전트의 작업을
**사무실처럼 생긴 화면에서, 귀엽게 움직이며, 서로 협업하는 모습**으로 보여 주는 도구를 찾았다.

## 한 줄 결론

- **협업(지시·일 맡기기)이 가장 잘 보이는 것**: [claude-office (paulrobello)](https://github.com/paulrobello/claude-office). 메인 Claude가 사장, 서브에이전트가 직원으로 나오고, 실제 Claude Code 훅으로 움직이며, Windows를 지원한다.
- **가장 많이 쓰이는 사무실형**: [Pixel Agents](https://github.com/pixel-agents-hq/pixel-agents). 스타 9,755개, VS Code 설치 약 9만 500회, Windows를 지원한다. 다만 일을 넘기는 장면은 그리지 않는다.
- **찾지 못한 것**: "A가 B에게 일을 넘기러 걸어간다"는 식으로 실제 인계와 회의를 작업 기록 그대로 그리는 도구는 없었다. 귀여운 회의·티타임 장면은 대부분 타이머로 도는 연출이다. 실제 데이터로 보여 주는 범위는 "누가 자리에서 무엇을 하고 있는가"까지다.
- 사용자가 보여 준 화면(팀별 방, 작업 카드, 일하는 사람·끝낸 요청·내 답변 필요 숫자, 팀 현황, 소식 피드, 한국어 화면)을 만든 도구는 찾지 못했다.

## 1. 협업이 보이는 사무실형

| 도구 | ⭐ | 최근 업데이트 | 협업을 어떻게 보여 주나 | 플랫폼 | 영상·데모 |
|---|---|---|---|---|---|
| [claude-office (paulrobello)](https://github.com/paulrobello/claude-office) | 567 | 10-16 | 사장(메인 Claude)이 직원(서브에이전트)을 뽑아 일을 맡긴다. "일 맡기는 중"과 "기다리는 중" 상태가 따로 보이고, 직원은 빈자리를 찾아 걸어가 앉는다. 백그라운드 작업은 화상회의처럼 "원격 근무자" 타일로 보인다. 실제 Claude Code 훅을 쓴다. | Windows·macOS·Linux | [데모 영상](https://youtu.be/AM2UjKYB8Ew) |
| [Agent Office (AgentSystemLabs)](https://github.com/AgentSystemLabs/agent-office) | 761 | 10-09 | 만화풍 3D 사무실이다. 에이전트가 다른 에이전트를 고용하고, 메시지를 보내고, 퇴근시킨다. GitHub 이슈를 직원에게 배정할 수 있고 터미널 공유와 화이트보드가 있다. 만든 사람이 "내 작업 방식에 맞춘 것이고 자주 바뀐다"고 적어 두었다. | Windows(PowerShell 설치)·macOS·Linux | 없음 |
| [Agent Office (howardguoui)](https://github.com/howardguoui/agent-office) | 0 | 10-09 | 서브에이전트가 생기면 책상이 하나 생기고, 에이전트끼리 일을 넘길 때 꾸러미가 날아가는 연출이 있다. "일 넘기기"를 눈으로 보여 주는 유일한 도구였다. | 브라우저(Node 18·Python 3.8) | [브라우저 데모](https://howardguoui.github.io/agent-office/) (실제 세션이 아닌 시뮬레이션) |
| [Claude-Office (W17ant)](https://github.com/W17ant/Claude-Office) | 244 | 10-16 | 아이소메트릭 사무실이다. 에이전트가 출근해 앉았다가 일이 끝나면 퇴근한다. 사무실 채팅 패널에 에이전트끼리 주고받는 메시지가 보이고, 권한 창이 뜨면 "Needs you" 알림이 나온다. | 명시 없음 | 없음 |
| [agent-office (samwang0723)](https://github.com/samwang0723/agent-office) | 4 | 03-12 | Claude Code Agent Teams(tmux 팀원 모드)를 픽셀 화면으로 보여 주고, 팀원에게 메시지를 보낼 수 있다. | 명시 없음 | 없음 |
| Outworked ([README](https://cdn.jsdelivr.net/gh/outworked/outworked@main/README.md)) | 392 | 04-01 | 역할을 정해 에이전트를 고용하면 총괄이 목표를 쪼개 나눠 준다. 에이전트는 걸어 다니고, 앉고, 협업한다. | macOS 중심(Windows는 시험 안 됨) | 없음 |
| [agent-room](https://github.com/lun79345-dotcom/agent-room) | 0 | 10-09 | Claude 팀장이 일을 쪼개 Codex 직원들에게 맡기는 단톡방 형태다. 사무실 그림은 없고 화면은 중국어다. | macOS 베타 | 없음 |

## 2. 귀엽게 움직이는 사무실형 (협업 표현은 약함)

| 도구 | ⭐ | 최근 업데이트 | 특징 | 플랫폼 | 영상·데모 |
|---|---|---|---|---|---|
| [Pixel Agents](https://github.com/pixel-agents-hq/pixel-agents) | 9,755 · 설치 약 9만 500회 | 10-17 | 터미널 하나가 캐릭터 하나다. 책상으로 걸어가 앉고, 파일을 고칠 때는 타자를 치고, 찾을 때는 책을 읽고, 대답이나 권한을 기다릴 때는 말풍선을 띄운다. 서브에이전트와 Agent Teams 팀원도 캐릭터로 나온다. 사무실 편집기와 애완동물이 있고, 폴더마다 구역을 정할 수 있다. 마켓에서는 게시자 `pablodelucca` 것을 고른다. | VS Code·`npx pixel-agents`(브라우저)·Windows·macOS·Linux | [데모](https://www.youtube.com/watch?v=dLmk2SqZXYo) · [실시간으로 보기](https://www.youtube.com/watch?v=KtG5Ez7KujE) · [설치부터](https://www.youtube.com/watch?v=QwFgLbI0fmI) · [독일어 쇼츠](https://www.youtube.com/shorts/ZxEaL0skQNo) · [스페인어](https://www.youtube.com/watch?v=luhy_RfYeX4) · [태국어](https://www.youtube.com/watch?v=3AfkDKoLQB4) |
| [Agent Virtual Office (KbWen)](https://github.com/KbWen/agent-virtual-office) | 46 | 10-11 | 사무실 생활 연출이 가장 많다. 직군마다 도구 사용에 따라 다른 자리로 간다(QA는 돋보기, 운영은 배포 버튼, 디자이너는 화이트보드). 스탠드업, 티타임, 버그 말다툼, 강아지 방문 같은 장면이 있다. 각자의 상태는 실제 훅에서 오지만, 사교 장면은 타이머로 도는 연출이다. | Node 22(`npx github:KbWen/agent-virtual-office`), Windows 명시 없음 | [움직임 GIF](https://raw.githubusercontent.com/KbWen/agent-virtual-office/main/docs/assets/office-poke.gif) |
| [Star Office UI](https://docsearch.algolia.com/mcp/docs/repo/ringhyacinth/star-office-ui) | 7,524 | 03-11 | 귀여운 픽셀 사무실과 데스크톱 펫 모드가 있다. OpenClaw 에이전트용이다. | Python 3.10 | [튜토리얼](https://www.youtube.com/watch?v=GMfcoRB4ki8) |
| [clawd-on-desk](https://gittrend.io/repo/rullerzhou-afk/clawd-on-desk) | 6,467 | 10-17 | 사무실이 아니라 화면 구석에 떠 있는 Claude 펫 하나다. 작업 상태에 따라 12가지 동작을 하고 권한 요청 말풍선을 띄운다. | Windows 11(winget)·macOS | [쇼츠](https://www.youtube.com/shorts/iUASpN-TV6o) |
| [The Agents Hub](https://github.com/IronLain88/The-Agents-Hub) | 조회 안 됨 | — | 에이전트가 작업대 사이를 걸어 다니고 게시판을 확인하며, 우편함·게시판으로 메시지를 주고받는다. 에이전트가 MCP를 직접 불러 자기 상태를 알리는 방식이라 자동으로 잡히지는 않는다. | Docker | 없음 |
| [pixel-agents-standalone](https://github.com/rolandal/pixel-agents-standalone) | 54 | 08-12 | Pixel Agents를 VS Code 없이 웹 앱으로 돌리는 포크다. | 브라우저 | 없음 |
| [Agent Office (alminisl)](https://github.com/alminisl/agent-office) | 6 | 09-30 | 세션마다 칸막이 자리가 있고, "Needs you: approve npm test"처럼 사람이 필요한 순간을 표시한다. | 명시 없음 | 없음 |
| [PixelOffice (KalebKE)](https://github.com/KalebKE/PixelOffice) | 12 | 08-01 | Claude Code와 Codex를 함께 보여 준다. | 명시 없음 | 없음 |
| Agent Office (영상만 찾음) | — | — | 팀이 스스로 커지고, 인턴을 뽑고, 서로 일을 맡긴다고 소개된다. 실제 코딩 에이전트에 연결되는지는 확인하지 못했다. | — | [영상](https://www.youtube.com/watch?v=GgrK8K9RlIA) |
| Claw3D | — | — | 3D 가상 사무실이다. Hermes 에이전트용이다. | — | [영상](https://www.youtube.com/watch?v=4BtY3EAnai0) |

## 3. 대시보드형 (사무실 그림 없음, 실무용)

| 도구 | ⭐ | 최근 업데이트 | 특징 | 영상·데모 |
|---|---|---|---|---|
| [Claude-Code-Agent-Monitor](https://github.com/hoangsonww/Claude-Code-Agent-Monitor) | 1,082 | 10-17 | 칸반 상태판, 부모·자식 에이전트 트리, 도구 흐름 그래프, 비용을 보여 준다. Windows 설치 프로그램이 있다. 훅과 대화 기록을 둘 다 읽는다. | 스크린샷만 있음 |
| [disler 멀티에이전트 관측](https://github.com/disler/claude-code-hooks-multi-agent-observability) | 1,548 | 02-08 | 이벤트 타임라인과 세션별 색 막대 차트를 보여 준다. 원조 격이라 포크가 많다. | [README 영상](https://youtu.be/RpUTF_U4kiw) · [I Can SEE EVERYTHING](https://www.youtube.com/watch?v=9ijnN985O_c) · [훅 해설](https://www.youtube.com/watch?v=J5B9UGTuNoM) · [워크트리 관측](https://www.youtube.com/watch?v=XMoIOykmxI8) |
| [agents-observe](https://github.com/simple10/agents-observe) | 698 | 09-04 | 도구 호출이 실시간으로 흐르고, 어떤 서브에이전트를 누가 띄웠는지 계층으로 보여 준다. Claude 플러그인으로 설치하며 Docker가 필요하다. | [데모 mp4](https://raw.githubusercontent.com/simple10/agents-observe/videos/docs/assets/demo.mp4) |
| Argus (VS Code 확장) | — | — | VS Code 안에서 에이전트 세션을 관측한다. | [영상](https://www.youtube.com/watch?v=HmHOI1PBn_M) |
| Claude Canvas | — | — | Claude Code 옆에 시각 화면을 하나 더 붙인다. | [영상1](https://www.youtube.com/watch?v=f09JZ6oKgeI) · [영상2](https://www.youtube.com/watch?v=zEaQFW3bblw) |
| OpenTelemetry + Grafana | — | — | 여러 에이전트의 토큰과 비용을 대시보드로 본다. | [영상](https://www.youtube.com/watch?v=Qu3XXSy0HtY) |
| c9watch · [claude-code-monitor](https://github.com/onikan27/claude-code-monitor) | — | — | 이 PC에서 도는 모든 세션을 찾아 보여 준다. claude-code-monitor는 macOS 전용이다. | — |

## 4. 병렬 작업 관리 도구 (보기보다 일을 나눠 주는 쪽)

| 도구 | ⭐ | 특징 | 영상 |
|---|---|---|---|
| [Vibe Kanban](https://github.com/BloopAI/vibe-kanban) | 28,384 | 칸반 보드에 카드를 올리면 에이전트가 하나씩 맡아 처리한다. | [영상1](https://www.youtube.com/watch?v=PgWHvLRaEAU) · [영상2](https://www.youtube.com/watch?v=kWlvet8fBS0) |
| [opcode](https://github.com/winfunc/opcode) | 22,409 | Claude Code 세션을 관리하는 GUI다. | — |
| [Claude Squad](https://github.com/smtg-ai/claude-squad) | 8,618 | 여러 에이전트를 터미널에서 나란히 관리한다. | — |
| Conductor | — | 격리된 작업 공간에서 Claude Code와 Codex를 여러 개 돌린다. Mac 전용이다. | [채널](https://www.youtube.com/@conductor_build) |

## 5. Claude Code 공식 기능

- **Agent view** (`claude agents`): 백그라운드 세션들이 무엇을 하는지, 무엇이 입력을 기다리는지, 무엇이 끝났는지를 한 화면에 보여 준다. 터미널 화면이다. [공식 소개](https://www.youtube.com/watch?v=-INveHwbRz4) · [튜토리얼](https://www.youtube.com/watch?v=Lj9F4Cts0ks) · [문서](https://code.claude.com/docs/en/agent-view)
- **Agent Teams + tmux**: 팀원 에이전트마다 창이 하나씩 열려 각자 일하는 모습이 보인다. 팀원끼리는 우편함과 공유 작업 목록으로 소통한다. 실험 기능이라 `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS`로 켠다. [영상1](https://www.youtube.com/watch?v=Dyj9ShddyMw) · [영상2](https://www.youtube.com/watch?v=pSeCCAkM7IA) · [한국어 설정법](https://www.youtube.com/watch?v=7-50cmRdHgA)
- **데스크톱 앱 분할 보기**: Code 탭에서 세션 두 개를 나란히 띄운다. [문서](https://code.claude.com/docs/en/desktop)

## 주의할 점

- 스타 수는 관심도이지 실제 사용자 수가 아니다. 실제 설치 수가 공개된 것은 VS Code 확장인 Pixel Agents뿐이다.
- 영상은 대부분 4~15개월 전 것이라 지금 버전과 다를 수 있다.
- Windows에서 확실히 쓸 수 있다고 적힌 것은 claude-office(paulrobello), Pixel Agents, Agent Office(AgentSystemLabs), Claude-Code-Agent-Monitor, clawd-on-desk다.
- 훅을 쓰는 도구는 설치할 때 `~/.claude/settings.json`에 훅을 추가한다. CL9는 이미 훅(`.claude/hooks/cl9_lock.py` 등)을 쓰고 있으니, 설치 전에 겹치지 않는지 확인해야 한다.

## 출처

본문 표의 링크 외에 다음을 참고했다.
[agent-visualization 토픽](https://github.com/topics/agent-visualization) ·
[Show HN: Claude Code 에이전트 팀 대시보드](https://news.ycombinator.com/item?id=47602986) ·
[DEV: Multi-Agent Observability](https://dev.to/bredmond1019/multi-agent-observability-see-everything-your-ai-agents-do-16e2) ·
[Agent Teams 조직 운영 글](https://claude-codex.fr/en/content/agent-team-orchestration/) ·
[Claude Code Agent Teams](https://cobusgreyling.substack.com/p/claude-code-agent-teams) ·
[Pixel Office JetBrains 플러그인](https://plugins.jetbrains.com/plugin/31298-pixel-office--ai-agent-visualizer) ·
[Hermes Pixel Office](https://www.smfclearinghouse.com/blog/2026-08-11-hermes-pixel-office-pixel-art-agent-dashboard/) ·
[AgentRoom](https://github.com/liuyixin-louis/agentroom) ·
[VS Code Marketplace: Pixel Agents](https://marketplace.visualstudio.com/items?itemName=pablodelucca.pixel-agents)
