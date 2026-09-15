# 훅 stdin 픽스처 (Claude Code 2.1.201, 2026-09-13 프로브)

`test_cl9_scope_guard.Fixtures` 가 읽는다. 형식: `{"stdin": <훅 stdin 그대로>, "expect": "allow|deny", "root_override": true}`.
`root_override` 는 stdin 의 cwd 대신 저장소 루트를 쓴다(캡처 머신 경로 무관).

캡처 환경: 훅 프로세스 체인 ['python.exe', 'py.exe', 'bash.exe', 'bash.exe', 'claude.exe', 'cmd.exe'], SHELL=C:\Program Files\Git\bin\bash.exe, MSYSTEM=MINGW64, CLAUDE_PROJECT_DIR=D:/Work/unreal/CL9, 실행 파일 C:\Users\user\AppData\Local\Programs\Python\Python39\python.exe.
