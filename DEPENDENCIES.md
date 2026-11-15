# 에디터 의존성 변경 안내 (2026-11-15)

에디터에 창 도킹과 이동/회전/크기 기즈모를 넣으면서 ImGui 관련 의존성이 바뀌었습니다.
받아서 빌드하기 전에 **"받은 뒤 해야 할 일"** 만 먼저 확인해 주세요.

## 한눈에 보기

| 항목 | 이전 | 지금 |
|---|---|---|
| Dear ImGui | 1.90.7 (master, vcpkg의 DLL) | **1.90.7 docking 브랜치** (`v1.90.7-docking`), 소스를 프로젝트에 포함 |
| ImGuizmo | 없음 | **새로 추가** (커밋 `ba662b1`, vcpkg 포트 기준 2024-05-29), 소스를 프로젝트에 포함 |
| ImGui를 가져오는 방법 | vcpkg `imgui` 패키지를 링크 (`imguid.lib` / `imgui.lib`) | `ThirdParty/` 아래 소스를 `DearsGraphicsEngine` 프로젝트가 직접 컴파일 |
| assimp, DirectXTK, DirectXTex | vcpkg | **변경 없음** |

ImGui의 버전 번호(1.90.7)는 같습니다. 바뀐 것은 브랜치입니다. docking 브랜치는 master에 창 도킹 기능이 더해진 것이라
기존 ImGui 코드는 고치지 않아도 그대로 컴파일됩니다.

## 새로 생긴 폴더

```
ThirdParty/
  imgui/              Dear ImGui v1.90.7-docking (MIT, LICENSE.txt 포함)
    backends/         imgui_impl_dx11, imgui_impl_win32
  ImGuizmo/           ImGuizmo (MIT, LICENSE 포함)
```

- 원본 소스를 포함하며, 커밋 공백 검사에 맞춘 일부 줄 끝 공백 정리 외에 기능 변경은 없습니다.
- 출처: `https://github.com/ocornut/imgui` 태그 `v1.90.7-docking`,
  `https://github.com/CedricGuillemet/ImGuizmo` 커밋 `ba662b119d64f9ab700bb2cd7b2781f9044f5565`.

## 프로젝트 설정에서 바뀐 것

- `DearsGraphicsEngine.vcxproj`: `ThirdParty`의 ImGui 소스 7개와 `ImGuizmo.cpp`를 빌드에 추가.
- `DearsGraphicsEngine.vcxproj`, `GameEngine.vcxproj`, `Demo.vcxproj` (x64 Debug/Release):
  추가 포함 디렉터리 맨 앞에 아래 세 경로를 추가.
  ```
  $(SolutionDir)ThirdParty\imgui
  $(SolutionDir)ThirdParty\imgui\backends
  $(SolutionDir)ThirdParty\ImGuizmo
  ```
  그래서 `#include <imgui.h>`, `<imgui_impl_dx11.h>`, `<ImGuizmo.h>`는 모두 `ThirdParty` 쪽을 가리킵니다.

## 받은 뒤 해야 할 일

1. **vcpkg의 ImGui를 링크하지 않게 하기 (중요)**
   ImGui가 이제 `DearsGraphicsEngine.lib` 안에 들어 있습니다. 예전처럼 `imguid.lib`/`imgui.lib`도 같이 링크하면
   실행 파일 안에 ImGui가 두 벌 생기고, 서로 다른 ImGui 컨텍스트를 쓰게 되어 UI가 안 뜨거나 크래시가 납니다.
   - 링크 입력에 `imguid.lib` / `imgui.lib`를 직접 적어 둔 곳이 있으면 지워 주세요.
     (이 작업을 한 PC에서는 로컬 파일 `Directory.Build.targets`의 `DearsDependencyLibraries`에서 뺐습니다.
     이 파일과 `vcpkg.json`, `Directory.Build.props`는 저장소에 올라가 있지 않은 로컬 파일입니다.)
   - `vcpkg integrate install`로 자동 링크를 쓰고 있다면, vcpkg에서 `imgui` 패키지를 제거하는 것이 가장 확실합니다.
   - 확인 방법: 빌드한 `Demo.exe`의 의존 DLL에 `imguid.dll`/`imgui.dll`이 **없어야** 정상입니다.
     ```
     dumpbin /dependents x64\Debug\Demo.exe
     ```
2. **vcpkg를 다시 설치할 필요는 없습니다.** assimp, DirectXTK, DirectXTex는 그대로입니다.
3. **`Demo/imgui.ini`**: 도킹 배치가 이 파일에 저장됩니다. 예전 파일이 있어도 문제없이 열리고,
   배치가 이상하면 메뉴의 `Window > Reset Layout`을 누르거나 파일을 지우면 기본 배치로 돌아갑니다.

## 왜 vcpkg 기능 추가가 아니라 소스 포함인가

처음에는 `vcpkg.json`의 imgui에 `docking-experimental` 기능을 추가하는 쪽으로 시도했습니다.
그런데 매니페스트를 바꾸면 vcpkg가 assimp까지 전부 다시 빌드하려 하고, 그 하위 의존성인
`jhasse-poly2tri`가 현재 CMake와 맞지 않아 빌드에 실패했습니다(임시 폴더에서 시험해서 프로젝트는 건드리지 않았습니다).
기존 라이브러리를 깨뜨릴 위험이 있어서, ImGui와 ImGuizmo만 소스로 포함하는 방식으로 정했습니다.
둘 다 원래 "소스를 프로젝트에 넣어 같이 컴파일"하도록 만들어진 라이브러리이고,
이 방식이면 팀원마다 vcpkg 상태가 달라도 같은 버전으로 빌드됩니다.

## 테스트 스크립트 참고

`Tests/AStarEditor`와 `Tests/EditorViewport`의 스크립트는 여전히 `vcpkg_installed` 안의 예전 ImGui(1.90.7 master)
소스로 컴파일합니다. 엔진 본체와 다른 ImGui를 쓰지만 서로 독립된 실행 파일이라 영향은 없고, 현재 모두 통과합니다.
vcpkg에서 imgui를 제거하면 이 두 스크립트의 경로를 `ThirdParty/imgui`로 바꿔야 합니다.

## 물리 엔진 작업에 쓸 수 있는 것

디버그 UI나 시각화를 붙일 때 아래를 그대로 쓸 수 있습니다.

- **도킹되는 디버그 창 만들기**: `IEditorPanel`을 상속해 `Draw()`에서 `ImGui::Begin("Physics") ... ImGui::End()`를 그리고
  `DearsGraphicsEngine::AddEditorPanel()`로 등록하면 `Window` 메뉴에 자동으로 나오고 다른 창에 붙일 수 있습니다.
  기본 자리를 정하려면 `SetEditorDockSlot("Physics", EditorDockSlot::Bottom)`처럼 지정합니다.
  (`GameEngine/RenderPipelinePanel.h`가 가장 짧은 예시입니다.)
- **기즈모**: `ImGuizmo::Manipulate(view, projection, operation, mode, matrix)`로 임의의 4x4 행렬을 뷰포트에서 조작할 수 있습니다.
  콜라이더 크기나 조인트 위치 편집에 쓸 수 있습니다. 사용 예는 `GameEngine/DemoSceneEditor.cpp`의 `DrawTransformGizmo()`입니다.
  - SimpleMath `Matrix`는 변환 없이 `&matrix._11`을 그대로 넘기면 됩니다.
  - `ImGuizmo::BeginFrame()`은 엔진이 매 프레임 호출하므로 다시 부르지 않습니다.
- **주의**: 애니메이션 시간과 데모 입력은 이제 **Play 중에만** 진행됩니다. 물리 시뮬레이션도 같은 기준
  (`IScene::IsPlaying()`)에 맞추면 편집 중에는 멈추고, Stop을 누르면 씬이 Play 직전 상태로 복원됩니다.
