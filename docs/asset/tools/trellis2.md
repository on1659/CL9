# TRELLIS.2 — CL9 로컬 생성 도구

작성·실행 환경 확인: 2026-10-12. 이 문서는 **모든 에셋에 공통인 TRELLIS 실행법의 정본**이다.
모델의 외형·입력 이미지·채택 여부·다음 작업은 [모델별 제작 카드](../README.md)에 기록한다.
닭 탈의 실제 제작 지시는 [chicken_disguise 인계서](../monster/chicken_disguise-animation-handoff.md)를 따른다.

## 1. 검증된 설치와 범위

| 항목 | 확인 값 |
|---|---|
| 설치 폴더 | `C:/Work/Tools/TRELLIS2` — CL9 저장소 밖의 이 컴퓨터 전용 설치 |
| 사용 소스 | [IgorAherne Windows/low-VRAM 포크](https://github.com/IgorAherne/TRELLIS.2-stableprojectorz) |
| 설치 기준 커밋 | `d5d38f1e033a881e2c00e3137822ef8aa4e193de` + 아래 로컬 수정 |
| 원 모델 | [Microsoft TRELLIS.2](https://github.com/microsoft/TRELLIS.2), `microsoft/TRELLIS.2-4B` |
| Python | `.venv/Scripts/python.exe`, 3.11.9 |
| 핵심 패키지 | PyTorch 2.8.0+cu128, xformers 0.0.32.post2 |
| 검증 GPU | RTX 5060 Laptop GPU, VRAM 8GB |
| 패키지 목록 | 설치 폴더의 `requirements-cl9-installed.txt` |
| 검증 경로 | `cl9_generate.py` 배치 생성 → GLB → 별도 Blender에서 임포트·렌더 |
| 기본 프로필 | 512 / seed 42 / 각 샘플러 12 steps / 목표 100,000 triangles / texture 2048 |

설치와 본체 생성은 성공했다. **1024_cascade, 다른 GPU, Gradio 전체 UI는 이 프로필과 같은 수준으로 검증되지 않았다.**
TRELLIS는 이미지에서 정지 메시와 재질을 만드는 단계다. 변형에 적합한 토폴로지, 뼈, 웨이트,
속 빈 의상 구조, 접촉과 애니메이션을 자동 완성했다고 간주하지 않는다.

## 2. 작업 시작 전

1. 카드의 기준 이미지와 실제 파일을 직접 연다. 배경·잘린 손발·여러 인물이 섞인 그림을 무검토로 넣지 않는다.
2. 입력의 SHA256, 출력 폴더, seed·resolution·steps·faces·texture를 기록한다.
3. 설치와 모델 캐시가 있는지 확인한다. 이미 성공한 환경에 `install.py`나 패키지 전체 업그레이드를 먼저 실행하지 않는다.
4. GPU·RAM 여유를 확인하고 생성은 한 번에 하나만 실행한다. 이 환경은 CPU로 모델을 내려 쓰므로 VRAM 외에 시스템 RAM도 필요하다.
5. 다른 앱/다른 세션의 저장하지 않은 작업을 임의로 종료하지 않는다. Unreal을 조작할 경우 [공유 세션 규약](../../workflow/sessions.md)을 따른다.
6. 입력이나 생성 조건을 바꾸면 **새 출력 폴더**를 쓴다. 기존 작업은 검수·비교 자료로 보존한다.

다음은 PowerShell에서 실행하는 읽기 전용 환경 확인 예다.

```powershell
$trellisRoot = 'C:/Work/Tools/TRELLIS2'
$trellisPython = "$trellisRoot/.venv/Scripts/python.exe"
& $trellisPython -c "import sys, torch; print(sys.version); print(torch.__version__); print(torch.cuda.is_available()); print(torch.cuda.get_device_name(0))"
nvidia-smi --query-gpu=name,memory.used,memory.free --format=csv
```

## 3. 임의의 모델 생성

아래 두 경로를 모델 카드에 적힌 실제 경로로 바꾼다. placeholder 상태로 실행하지 않는다.
이 예는 **빈 출력 폴더에서 새 생성**을 시작한다. 중단 재개는 다음 절을 먼저 읽는다.

```powershell
$trellisRoot = 'C:/Work/Tools/TRELLIS2'
$trellisPython = "$trellisRoot/.venv/Scripts/python.exe"
$sourceImage = 'C:/path/to/reference.png'
$outputDir = 'C:/path/to/new-output-directory'

& $trellisPython -c "import pathlib,sys,hashlib; s=pathlib.Path(sys.argv[1]); p=pathlib.Path(sys.argv[2]); assert s.is_file(), 'input missing'; assert not p.exists() or (p.is_dir() and not any(p.iterdir())), 'output is not empty: inspect existing result'; p.mkdir(parents=True,exist_ok=True); print(hashlib.sha256(s.read_bytes()).hexdigest())" $sourceImage $outputDir
if ($LASTEXITCODE -ne 0) { throw '입력/출력 확인 실패. 생성하지 않습니다.' }

& $trellisPython -u "$trellisRoot/cl9_generate.py" `
  --image $sourceImage --output $outputDir `
  --resolution 512 --seed 42 --steps 12 --faces 100000 --texture 2048 `
  *> "$outputDir/generate.log"
if ($LASTEXITCODE -ne 0) { throw 'TRELLIS 생성 실패. generate.log와 generation.json을 확인하세요.' }
```

`--faces`는 후처리 목표이며 정확한 출력 개수를 보장하지 않는다. 실제 삼각형 수는 Blender에서 다시 측정한다.
`run-cl9-monster.ps1`은 **본체 이미지 경로가 고정된 래퍼**다. 닭이나 다른 모델에는 위의 `cl9_generate.py --image ...`를 쓴다.

## 4. 산출물과 중단 재개

| 파일 | 의미 |
|---|---|
| `generation.json` | 입력 해시·설정·실행 단계·시간. 실행 도중 갱신됨 |
| `input_preprocessed.png` | 배경 제거/입력 전처리 결과. 손발·볏·날개 누락을 직접 확인 |
| `pipeline_config/pipeline.json` | 해당 실행에 선택된 모델 구성 |
| `latents.pt` | 생성 중간 결과. 같은 조건 재시도에서 샘플링 단계를 건너뜀 |
| `raw_mesh.pt` | GLB 후처리 전 메시/재질 데이터. 자동 재개 입력은 아님 |
| `model.glb` | 최종 정지 모델. 존재만으로 임포트·외형 검수 성공을 뜻하지 않음 |
| `generate.log` | 위 명령으로 생성하는 로그 |

**현재 runner의 한계:** `input_preprocessed.png`와 `latents.pt`가 있으면 기존 파일을 재사용하지만,
그 파일들이 새 입력/seed/설정에 맞는지 자동 검사하지 않는다. `generation.json`도 실행 초기에 다시 쓴다.

중단 후에는 **재실행 전에** 이전 `generation.json`과 입력 해시, 모든 옵션을 비교·보존한다.
같은 입력과 같은 조건이고 GLB가 아직 없을 때만 같은 폴더로 재개한다. 조건이 다르면 새 폴더로 시작한다.
`model.glb`가 이미 있으면 runner가 덮어쓰기를 거부한다. 재실행하려고 성공한 GLB를 지우지 않는다.
기존 GLB를 Blender에서 먼저 확인하고, 새 후보가 필요하면 버전 폴더를 추가한다.

## 5. 이 GPU를 위한 로컬 수정 — 유지할 것

- `pipeline_worker.py`의 `_apply_patches()`: Blackwell에서 xformers forward를 검증된 CUTLASS로 선택.
  기본 선택의 Hopper Flash3가 이 GPU에서 `invalid argument`로 실패한 이력이 있다.
- `cl9_generate.py`: dense attention은 PyTorch SDPA, sparse attention은 xformers.
- flow model의 transformer block을 하나씩 GPU에 올렸다 CPU로 내린다. decoder 전체에 같은 hook을 붙이지 않는다.
- 512 실행에서 쓰지 않는 1024 flow model은 로드하지 않는다. decoder는 포크의 low-VRAM 경로를 유지한다.
- 캐시는 설치 폴더의 `models/`, 보조 모델은 `MODELS/`에 있다. 전역 Python과 Windows 보안 설정을 바꾸지 않았다.

검증된 본체 실행에서 로드·생성·GLB 저장까지 약 296초가 걸렸다. 설치/다운로드 시간은 제외하며,
다른 모델·다른 앱 점유 상태의 소요 시간을 보장하지 않는다. 전체 GPU 점유와 PyTorch 할당량도 구분한다.

## 6. 실패 처리

| 증상 | 확인과 조치 |
|---|---|
| CUDA `invalid argument`, Flash/Hopper 스택 | 올바른 venv/로컬 패치인지 확인. 검증된 SDPA + sparse CUTLASS 경로로 복구 |
| 매우 느림 / GPU 메모리 부족 | GPU와 시스템 RAM 점유, 동시 생성 여부, block offload 로그 확인. 먼저 512 사용 |
| `input_preprocessed.png`에서 볏·손발 소실 | 원본과 직접 비교. 입력을 고쳤다면 새 폴더에서 전처리·생성 |
| 다른 이미지를 넣어도 이전 형상 | 중간 파일 재사용 여부 확인. 새 출력 폴더 사용 |
| GLB 내보내기만 실패 | 로그·latents·raw_mesh 보존. 같은 조건에서 재개하거나 raw mesh 후처리 경로를 별도 작성·검증 |
| 기존 GLB 존재 오류 | 정상적인 덮어쓰기 방지. 기존 결과 검수 또는 새 후보 경로 지정 |
| 모듈/가중치 누락 | 설치 폴더·venv·캐시 확인. 설치 로그와 패키지 목록 기준으로 누락 항목만 복구 |

새 컴퓨터에서는 이 경로가 존재한다고 가정하지 않는다. 위 저장소·커밋·패키지 목록과 로컬 패치를 먼저 확보한다.
`cl9_generate.py`와 로컬 패치는 현재 이 설치에만 있으며, CL9를 clone하는 것만으로 재현되지 않는다.
환경을 옮길 때는 스크립트·수정 diff·패키지 목록·가중치 출처를 함께 기록하고 짧은 생성부터 검증한다.
가중치와 native wheel을 출처 확인 없이 다른 배포본으로 바꾸지 않는다.

## 7. Blender에서 이어가기

생성 GLB를 보존하고 작업 사본에서 [공통 규격](../common.md)과 [제작·검수 과정](../workflow.md)을 적용한다.
축·실제 크기·바닥 원점은 생성기가 프로젝트 기준을 맞췄다고 가정하지 말고 리깅 전에 확정한다.
정면·측면·뒤·3/4를 렌더하고, 생성 오류와 보이지 않던 뒤쪽의 추정 형상을 기록한다.
관절을 구부릴 모델은 변형용 메시를 정리하고, 의상은 필요한 내부·입구·두께를 따로 설계한다.

설치 폴더의 `cl9_preview_blender.py`는 **기존 본체 전용 프리뷰**다. 이름과 높이 3.9m가 고정되어 있다.
닭/다른 모델에 그대로 적용해 크기와 이름을 확정하지 않는다. 해당 모델에 맞는 별도 프리뷰 스크립트를 만든다.
`cl9_compare_monsters.py`와 `cl9_compare_sheet.py`도 두 본체 후보의 고정 비교용이며 범용 제작 도구가 아니다.

`run-ui.ps1`은 upstream Gradio를 `127.0.0.1`에 여는 보조 경로다. CL9 batch runner의 block offload가
그 UI에 자동 적용되는 것은 아니며, 이 컴퓨터의 기본 생성 경로는 검증된 batch runner다.

## 8. 기록 위치

도구 설치·공통 실행법은 이 문서만 갱신한다. 개별 모델 카드에는 입력/출력 경로·설정·선택 근거·결과만 적고 링크한다.
실제 실행 로그·manifest·메시/영상 검수 증거는 `ArtSource/<분류>/<asset_id>/<작업버전>/`에 둔다.
출처·라이선스는 소스/가중치/입력 이미지별로 확인·기록하며, 이 문서가 상업 사용 권리 검토를 대신하지 않는다.
