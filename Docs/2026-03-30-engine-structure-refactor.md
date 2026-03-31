# 2026-03-30 엔진 구조 정리 작업 기록

## 작업 목표

오늘 작업의 목표는 `GameEngine`에 과하게 몰려 있던 초기화, 업데이트, 데모 월드 구성을 정리해서
엔진 코어와 샘플 씬의 책임을 더 명확하게 분리하는 것이었다.

정리 방향은 크게 네 가지였다.

1. `GameEngine::Initialize()`와 `GameEngine::Update()`의 거대한 절차를 역할별 함수로 분리한다.
2. 데모 전용 오브젝트 생성과 에디터 패널 등록을 별도 씬 객체로 이동한다.
3. 데모 전용 입력과 프레젠테이션 제어를 `GameEngine`에서 걷어내고 씬 쪽 책임으로 보낸다.
4. 싱글톤 매니저 수명 정리를 보강해서 종료 시 정리 흐름을 명시한다.

---

## 변경한 주요 파일

- `D:\DearsEngine\GameEngine\GameEngine.h`
- `D:\DearsEngine\GameEngine\GameEngine.cpp`
- `D:\DearsEngine\GameEngine\DemoScene.h`
- `D:\DearsEngine\GameEngine\DemoScene.cpp`
- `D:\DearsEngine\GameEngine\InputManager.cpp`
- `D:\DearsEngine\GameEngine\TimeManager.h`
- `D:\DearsEngine\GameEngine\TimeManager.cpp`
- `D:\DearsEngine\GameEngine\GameEngine.vcxproj`

---

## 1. Initialize 흐름 분리

기존에는 `GameEngine::Initialize()` 안에 자산 로드, 카메라 생성, 오브젝트 생성, 패널 초기화,
조명 세팅이 한 함수에 몰려 있었다.

이 흐름을 아래 helper 함수들로 분리했다.

- `InitializeManager()`
- `LoadDemoAssets()`
- `InitializeCameras()`
- `CreateSceneObjects()`
- `InitializeEditorPanels()`
- `InitializeLighting()`

이렇게 분리한 이유는 다음과 같다.

- 초기화 순서를 한눈에 읽을 수 있다.
- 어떤 단계가 실패하거나 수정이 필요할 때 진입점을 바로 찾을 수 있다.
- 샘플 씬 생성과 엔진 공통 초기화를 물리적으로 분리하기 쉬워진다.

현재 `Initialize()`는 "매니저 -> 렌더러 -> 자산 -> 카메라 -> 씬 -> 패널 -> 조명" 순서를 명확히 보여준다.

---

## 2. Update 흐름 분리

기존 `GameEngine::Update()`는 입력 처리, 카메라 이동, 데모 조작, 조명 상수 갱신, 씬 오브젝트 업데이트,
프레젠테이션 테스트가 한 함수에 섞여 있었다.

이 부분도 역할별 함수로 나눴다.

- `UpdateInputState()`
- `UpdateCameraControls(float deltaTime)`
- `UpdateDemoControls()`
- `UpdateLightingState()`
- `UpdateSceneObjects(float deltaTime)`
- `UpdatePresentationControls()`

이렇게 바꾸면서 얻은 점은 다음과 같다.

- 엔진 루프의 상위 흐름이 더 잘 보인다.
- 카메라 입력과 데모 입력이 섞이지 않는다.
- 이후 `Scene` 계층을 더 확장할 때 옮겨야 할 로직이 눈에 들어온다.

---

## 3. DemoScene 클래스 도입

샘플 월드 구성 로직을 `GameEngine`에서 분리하기 위해 `DemoScene`을 새로 만들었다.

추가한 파일:

- `D:\DearsEngine\GameEngine\DemoScene.h`
- `D:\DearsEngine\GameEngine\DemoScene.cpp`

`DemoScene`이 맡도록 정리한 책임은 다음과 같다.

- 샘플 씬 오브젝트 생성
- 샘플 씬 오브젝트 업데이트
- 씬 계층 패널과 인스펙터 패널 등록
- 데모 전용 입력 처리
- 데모 전용 프레젠테이션 조작
- 데모 전용 렌더 테스트 입력 처리

즉 `GameEngine`은 "엔진 루프와 렌더 흐름을 관리하는 곳"으로 남기고,
`DemoScene`은 "현재 샘플 월드가 어떤 내용으로 구성되는지"를 담당하게 만들었다.

---

## 4. DemoScene으로 옮긴 씬 구성 책임

기존에는 `GameEngine`이 직접 여러 `RenderObject`를 생성하고,
각 오브젝트를 어떤 의미로 쓰는지도 알고 있었다.

이제는 그 역할을 `DemoScene::CreateSceneObjects()`가 담당한다.

현재 씬에서 관리하는 대표 오브젝트는 다음과 같다.

- Character
- Weapon
- Billboard
- PBR Sphere
- Floor
- Water
- Terrain
- CubeMap

`DemoScene`은 내부적으로 이 오브젝트들을 보관하고,
`GameEngine`에는 `GetCharacter()`, `GetWeapon()`, `GetTerrain()` 같은 명시적 접근자만 제공한다.

이 구조의 장점은 다음과 같다.

- `GameEngine`이 샘플 월드의 내부 컨테이너 구조를 몰라도 된다.
- 나중에 `TestScene`, `BattleScene` 같은 다른 씬으로 확장하기 쉬워진다.
- 샘플 월드 변경이 엔진 코어 파일에 직접 번지지 않는다.

---

## 5. 에디터 패널 등록도 DemoScene으로 이동

기존에는 씬 계층 패널과 오브젝트 인스펙터 패널 등록도 `GameEngine`이 직접 담당했다.

이제는 `DemoScene::RegisterEditorPanels(int screenWidth)`가 담당한다.

이 함수 안에서 처리하는 일은 다음과 같다.

- `SceneHierarchyPanel` 생성
- 각 오브젝트 이름 등록
- `ObjectInspectorPanel` 생성
- 그래픽 엔진에 패널 등록
- 렌더 뷰포트 너비 설정

이렇게 바꾸면 패널 구성도 씬의 일부로 볼 수 있어서,
샘플 씬이 바뀌어도 엔진 코어를 덜 건드리게 된다.

---

## 6. 데모 입력 제어를 DemoScene으로 이동

오늘 구조 정리에서 가장 중요한 마무리 작업 중 하나는
데모 전용 입력을 `GameEngine` 밖으로 빼는 것이었다.

새로 만든 DemoScene 입력 함수는 다음과 같다.

### `HandleDemoInput(InputManager& inputManager)`

담당 기능:

- `W`, `A`, `S`, `D` 로 파티클 스폰 위치 이동
- `F` 로 테스트 파티클 생성
- `1`, `2`, `3` 으로 큐브맵 교체

이 로직은 엔진 공통 기능이 아니라 샘플 씬 실험 기능이므로
`GameEngine`보다 `DemoScene`에 있는 편이 자연스럽다.

### `HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime)`

담당 기능:

- `I`, `O` 로 빌보드 mipmap 레벨 조절
- `4`, `5` 로 UI 트윈 테스트
- `9`, `0` 으로 opacity와 thin film 관련 값 테스트

이 부분도 역시 엔진 전체 규칙이 아니라 샘플 연출 테스트에 가깝기 때문에
씬 쪽으로 옮겼다.

### `HandleRenderInput(InputManager& inputManager)`

담당 기능:

- `U` 로 PBR Sphere 회전
- `K`, `L` 로 PBR height scale 조절

렌더 직전의 샘플 오브젝트 조작도 `DemoScene`에서 관리하도록 정리했다.

---

## 7. GameEngine에서 걷어낸 잔여 상태 정리

구조를 옮기면서 `GameEngine`에 남아 있던 샘플 전용 상태들도 제거하거나 축소했다.

대표적으로 정리한 내용은 다음과 같다.

- `uiPoint`, `uiPoint2` 같은 샘플 UI 좌표를 `DemoScene` 쪽 상태로 고정
- `m_uiTweenTime`, `m_demoParticleSpawnPos`, `opacityValue` 같은 샘플 전용 값을 `DemoScene`으로 이동
- `SpawnDemoParticle()`를 `DemoScene` 내부 함수로 이동
- `InitializeLighting()`에 남아 있던 샘플 UI 좌표 초기화 제거

즉 `GameEngine`은 더 이상 샘플 UI 애니메이션 상태나 파티클 테스트 좌표를 직접 들고 있지 않는다.

---

## 8. 싱글톤 매니저 수명 정리

입력과 시간 매니저는 싱글톤으로 쓰이고 있었는데,
종료 시 정리 흐름이 충분히 드러나지 않았다.

오늘 정리한 부분은 다음과 같다.

### InputManager

- `getInstance()`에서 중복 초기화가 일어나던 흐름 수정
- `destroyInstance()`에서 `Finalize()` 호출 후 삭제하도록 정리
- `Initialize()`에서 내부 키 상태 배열을 다시 초기화하도록 보강

### TimeManager

- `destroyInstance()` 추가
- 종료 시 인스턴스를 명시적으로 정리할 수 있도록 개선

### GameEngine::Finalize()

- `m_pDemoScene.reset()`으로 씬을 먼저 정리
- 이후 `InputManager::destroyInstance()` 호출
- 이후 `TimeManager::destroyInstance()` 호출
- 내부 포인터를 `nullptr`로 정리

이 순서로 정리하면 엔진 종료 시 자원 수명이 좀 더 읽기 쉽고 예측 가능해진다.

---

## 9. 주석 보강 방향

오늘 사용자 요청에 맞춰 우리가 손댄 구조 변경 지점에는
담백한 설명 주석을 추가했다.

주석 원칙은 다음처럼 잡았다.

- 이모지 사용 금지
- 과한 장식 표현 금지
- 함수가 왜 분리되었는지 설명
- "엔진 공통 책임"과 "샘플 씬 책임"의 경계를 드러내기

특히 아래 위치에 설명 주석을 보강했다.

- `GameEngine::Initialize()`
- `GameEngine::CreateSceneObjects()`
- `GameEngine::InitializeLighting()`
- `GameEngine::Update()`
- `DemoScene`의 입력/프레젠테이션/렌더 제어 함수
- `DemoScene` 멤버 상태 설명

---

## 10. 빌드 검증 결과

빌드 대상:

- `D:\DearsEngine\Demo.sln`
- 설정: `Debug | x64`

결과:

- 빌드 성공
- 오류 0개

남아 있는 경고:

- `C4819`: 일부 파일 인코딩 경고
- `C4305`: `double` -> `float` 잘림 경고
- `C4244`: `float` -> `int` 변환 경고

추가로 vcpkg app-local 단계에서 `pwsh.exe` 관련 9009 메시지가 나오지만,
이후 `powershell.exe` 경로로 이어서 DLL 복사가 수행되어 최종 `Demo.exe`는 정상 생성됐다.

즉 현재 상태는 "구조 변경 후 실행 파일이 다시 만들어지는 빌드 가능 상태"로 확인했다.

---

## 11. 오늘 작업 후 구조 변화 요약

작업 전:

- `GameEngine`이 엔진 루프, 샘플 씬 구성, 샘플 입력, 프레젠테이션 테스트까지 대부분 직접 담당

작업 후:

- `GameEngine`
  - 엔진 루프 관리
  - 매니저 연결
  - 카메라 제어
  - 조명 상수 갱신
  - 렌더 순서 제어
  - 활성 씬 호출

- `DemoScene`
  - 샘플 월드 오브젝트 구성
  - 샘플 월드 업데이트
  - 샘플 패널 등록
  - 샘플 입력 처리
  - 샘플 프레젠테이션 조작
  - 샘플 렌더 테스트 조작

이제 `GameEngine`은 좀 더 "엔진 코어"처럼 보이고,
`DemoScene`은 "현재 샘플 월드"라는 성격이 분명해졌다.

---

## 12. 다음 작업 후보

다음으로 이어서 보기 좋은 작업은 아래 순서다.

1. 인코딩 경고가 나는 파일들을 UTF-8 기준으로 정리
2. `C4305`, `C4244` 경고를 단계적으로 정리
3. 씬 인터페이스를 공통 베이스 클래스로 뽑아 `DemoScene` 외 다른 씬도 붙일 수 있게 확장
4. `Render()` 내부의 샘플 UI 디버그 출력까지 장기적으로 씬 또는 별도 디버그 레이어로 분리

---

## 13. 추가 작업: 공통 씬 인터페이스 도입

후속 작업으로 `DemoScene`이 엔진 쪽에 구현체 이름 그대로 노출되는 문제를 줄이기 위해
공통 씬 인터페이스를 얇게 하나 도입했다.

추가 파일:

- `D:\DearsEngine\GameEngine\IScene.h`

이 인터페이스에는 현재 `GameEngine`이 실제로 필요로 하는 최소 기능만 넣었다.

- 초기화
- 에디터 패널 등록
- 씬 업데이트
- 데모 입력 처리
- 프레젠테이션 입력 처리
- 렌더 테스트 입력 처리
- 주요 렌더 오브젝트 접근자
- 샘플 UI 좌표 접근자

적용 방식은 다음과 같다.

- `DemoScene`이 `IScene`을 구현
- `GameEngine`은 `std::unique_ptr<IScene>` 형태로 현재 씬을 보관
- 실제 생성은 여전히 `DemoScene`이지만, 사용 지점은 인터페이스 기준으로 정리

이 변경으로 얻은 점은 다음과 같다.

- `GameEngine`이 특정 씬 구현체 이름에 덜 묶인다.
- 나중에 다른 테스트 씬을 추가할 때 교체 지점이 분명해진다.
- 엔진 루프와 씬 구현의 경계가 한층 더 선명해진다.

---

## 14. 추가 작업: 인코딩 경고 1차 정리

빌드 로그에서 가장 눈에 띄던 `C4819` 경고를 줄이기 위해
경고가 직접 발생하던 파일들을 UTF-8로 정리했다.

### 먼저 변환한 헤더

- `D:\DearsEngine\DearsGraphicsEngine\DearsGraphicsEngine.h`
- `D:\DearsEngine\DearsGraphicsEngine\Renderer.h`
- `D:\DearsEngine\DearsGraphicsEngine\GraphicsResourceManager.h`
- `D:\DearsEngine\DearsGraphicsEngine\DearsImGui.h`
- `D:\DearsEngine\GameEngine\RenderObject.h`

### 이어서 변환한 구현 파일

- `D:\DearsEngine\DearsGraphicsEngine\DearsGraphicsEngine.cpp`
- `D:\DearsEngine\DearsGraphicsEngine\DearsImGui.cpp`
- `D:\DearsEngine\DearsGraphicsEngine\GraphicsResourceManager.cpp`
- `D:\DearsEngine\DearsGraphicsEngine\Renderer.cpp`
- `D:\DearsEngine\GameEngine\RenderObject.cpp`

이번 정리 후 변화는 다음과 같다.

- `C4819` 경고는 빌드 결과에서 사실상 사라짐
- 남은 경고는 주로 실제 코드 문제에 가까운 항목들로 압축됨
  - `C4305`
  - `C4244`
  - `C4005`
  - `C4018`
  - `C4267`

즉 "파일이 어떤 인코딩이냐"보다 "코드가 어떤 타입 변환과 비교를 하고 있느냐"가
이제 빌드 로그의 중심이 되도록 정리했다는 의미가 있다.

---

## 15. 최신 빌드 결과

추가 정리 후 다시 빌드한 대상:

- `D:\DearsEngine\Demo.sln`
- 설정: `Debug | x64`

결과:

- 빌드 성공
- 오류 0개
- 경고 34개

이전보다 좋아진 점:

- `C4819` 경고가 정리되면서 로그가 훨씬 읽기 쉬워짐
- 경고 수가 이전 59개 수준에서 34개까지 줄어듦

현재 남아 있는 경고 성격:

- `Material.h` 계열의 `double -> float` 잘림
- `BufferData.h` 계열의 `float -> int` 변환
- `GraphicsResourceManager.cpp`의 매크로 재정의 및 signed/unsigned 비교
- `Renderer.cpp`, `DearsGraphicsEngine.cpp` 일부 숫자형 변환 경고

즉 다음 단계는 더 이상 인코딩보다
"실제 타입 안정성"과 "전처리기/비교식 정리" 쪽으로 넘어가면 된다.

---

## 16. 추가 작업: ImGui 패널 비율과 렌더 종횡비 보정

후속으로 확인한 문제는 오른쪽 ImGui 패널이 차지하는 폭 때문에
왼쪽 렌더 영역의 화면 비율이 어색하게 보인다는 점이었다.

기존 구조의 문제는 두 가지였다.

1. 패널 폭이 `380` 픽셀 고정값이라 해상도에 따라 상대 비율이 달라졌다.
2. 렌더 뷰포트 폭은 줄였지만 카메라 aspect ratio는 원래 전체 화면 폭 기준이라
   3D 화면이 눌리거나 늘어난 것처럼 보일 수 있었다.

이번에 적용한 수정은 다음과 같다.

### 패널 폭을 화면 비율 기준으로 변경

`D:\DearsEngine\GameEngine\SceneHierarchyPanel.h`에 아래 개념을 추가했다.

- `EDITOR_PANEL_WIDTH_RATIO = 1.0f / 6.0f`
- `GetEditorPanelWidth(float screenWidth)`

즉 전체 가로폭 중 `1/6`을 에디터 패널 영역으로 쓰고,
나머지 `5/6`을 렌더 영역으로 쓰도록 바꿨다.

이 함수는 아래 두 패널이 함께 사용한다.

- `SceneHierarchyPanel`
- `ObjectInspectorPanel`

두 패널은 세로로 나뉘어 쌓이지만 같은 폭을 공유하므로,
오른쪽 툴 영역이 화면 전체의 약 `1/6`만 차지하도록 맞췄다.

### DemoScene에서 렌더 뷰포트 폭을 새 비율로 설정

`D:\DearsEngine\GameEngine\DemoScene.cpp`의 `RegisterEditorPanels()`에서
고정 `EDITOR_PANEL_WIDTH` 대신 `GetEditorPanelWidth(screenWidth)`를 사용하도록 바꿨다.

즉 이제 렌더 뷰포트 폭은 다음 기준으로 계산된다.

- `renderViewportWidth = screenWidth - editorPanelWidth`

### GameEngine에서 카메라 aspect ratio도 함께 갱신

`D:\DearsEngine\GameEngine\GameEngine.cpp`의 `InitializeEditorPanels()`에서는
패널 등록 뒤 메인 카메라의 aspect ratio를 새 렌더 폭 기준으로 다시 설정한다.

이 단계가 필요한 이유는 다음과 같다.

- 뷰포트 폭만 줄이고 카메라 aspect를 그대로 두면
  3D 장면이 시각적으로 찌그러져 보일 수 있다.
- 렌더 폭과 카메라 aspect를 같이 맞춰야 화면비가 자연스럽다.

적용 후 의도한 비율은 다음과 같다.

- 렌더 영역: 약 `5`
- ImGui 툴 영역: 약 `1`

즉 사용자가 요청한 `5:1`에 가까운 구성이 되도록 반영했다.

---

## 17. 추가 작업: ImGui 입력 차단 문제와 폰트 크기, 한글 문자열 수정

비율을 조정한 뒤 실제 실행 화면을 확인하면서 세 가지 문제가 더 드러났다.

1. 오른쪽 ImGui 패널이 클릭되지 않음
2. 패널 폰트가 지나치게 큼
3. 화면 오버레이의 한글 문자열이 깨져 보임

각 문제는 서로 다른 원인에서 왔다.

### 17-1. 패널이 클릭되지 않던 원인

`D:\DearsEngine\DearsGraphicsEngine\DearsGraphicsEngine.cpp`를 보면
렌더 시작 시 전체 화면 크기의 `UI Canvas`를 하나 열고,
그 위에 텍스트와 이미지를 그리는 구조였다.

문제는 이 캔버스가 화면 전체를 덮고 있으면서
입력도 같이 받아버리고 있었다는 점이다.

그래서 오른쪽 패널이 시각적으로는 보이지만,
실제로는 뒤에 있는 창처럼 되어 클릭이 막히는 상태가 발생했다.

이 문제를 해결하기 위해
`D:\DearsEngine\DearsGraphicsEngine\DearsImGui.cpp`의 `UICanvasSet()`에서
`ImGuiWindowFlags_NoInputs`를 추가했다.

의도는 다음과 같다.

- `UI Canvas`는 단순한 오버레이 그리기 레이어로만 사용
- 입력은 `SceneHierarchyPanel`, `ObjectInspectorPanel`이 정상적으로 받도록 유지

### 17-2. 패널 폰트가 너무 크던 원인

에디터 패널 텍스트가 과하게 크게 보인 이유는
커스텀 한글 폰트 `NotoSansKR-Regular(48).ttf`가 사실상 UI 전체 기본 폰트처럼 쓰이고 있었기 때문이다.

이를 완화하기 위해 두 가지를 적용했다.

1. `D:\DearsEngine\DearsGraphicsEngine\DearsImGui.cpp`
   - ImGui 컨텍스트 생성 직후 `AddFontDefault()`를 추가
   - 에디터 패널은 우선 작은 기본 폰트를 쓰도록 정리

2. `D:\DearsEngine\GameEngine\SceneHierarchyPanel.h`
   `D:\DearsEngine\GameEngine\ObjectInspectorPanel.h`
   - 각 패널 창에서 `ImGui::SetWindowFontScale(0.8f)` 적용

즉 현재 구조는 다음처럼 나뉜다.

- 패널 기본 폰트: 작은 기본 ImGui 폰트
- 한국어 오버레이 텍스트: 필요할 때만 `NotoSansKR-Regular(48).ttf` 사용

### 17-3. 한글 문자열이 깨지던 원인

왼쪽 화면에 오버레이로 띄우던 텍스트는
`D:\DearsEngine\GameEngine\GameEngine.cpp`에 깨진 문자열 리터럴이 그대로 남아 있었다.

그래서 이 부분을 사람이 읽을 수 있는 한국어로 직접 교체했다.

수정한 대표 문자열:

- 조작 안내
  - `1~3: 큐브맵 변경`
  - `WASD: 파티클 위치 이동`
  - `F: 파티클 생성`
- 마우스 좌표
- 카메라 좌표
- 카메라 회전축 정보
- Light 카메라 좌표

즉 인코딩만의 문제가 아니라,
문자열 리터럴 자체가 이미 깨져 있던 부분을 실제 한국어 문장으로 복구한 셈이다.

### 17-4. 빌드 결과

수정 후 다시 빌드한 결과:

- `D:\DearsEngine\Demo.sln`
- `Debug | x64`
- 빌드 성공
- 오류 0개

중간에 한 번 `Demo.exe`가 실행 중이라 링크가 막혔지만,
실행 중 프로세스를 정리한 뒤 재빌드해서 최종 성공 상태를 확인했다.

즉 이번 단계까지 반영하면 기대 효과는 다음과 같다.

- 오른쪽 패널 클릭 가능
- 패널 폰트 크기 완화
- 왼쪽 한국어 안내 텍스트 정상 표시

---

## 마무리

오늘 작업으로 엔진 코어와 샘플 데모 사이의 경계가 한층 선명해졌다.
아직 완전한 씬 시스템이라고 보긴 이르지만,
`GameEngine`이 직접 모든 샘플 상태를 쥐고 있던 구조에서는 분명 한 단계 벗어난 상태다.

이 기반 위에서 다음에는 인코딩 정리와 공통 씬 인터페이스 쪽으로 이어가면
코드베이스를 더 장기적으로 관리하기 쉬워질 것이다.
