# YachtDice

Windows C++ 클라이언트 게임 프로젝트. DirectX 11 기반 커스텀 렌더링 엔진을 직접 구현합니다.



## 개요

1. 게임 'Yacht Dice'를 모방한 게임을 만듭니다. (예정)
2. IOCP, TCP 서버를 이용하여 송수신을 합니다. (예정)
3. 나만의 물리엔진 구현, 의도적인 물리적 버그를 활용하여 게임 컨텐츠에 활용합니다. (예정)



---

## 프로젝트 구조

```
YachtDice/
├── CoreLib/          # 메모리, 스레드, 잡큐, 동기화 등 엔진 코어
├── ServerCoreLib/    # IOCP 네트워크 (Listener, Session, Service)
├── RenderLib/        # DirectX 11 렌더링 엔진
├── GameEngineLib/    # 게임 엔진 (씬, 오브젝트, 컴포넌트, 리소스 매니저)
├── Client/           # 게임 클라이언트 진입점 및 씬/컴포넌트 구현
└── Tool/             # 모델 익스포터 및 에디터 (Assimp 사용)
```

**빌드 환경**
- Visual Studio 2022, Windows SDK, C++17
- 빌드: `msbuild Client\Client.vcxproj /p:Configuration=Debug /p:Platform=x64`
- PreBuildEvent: `UpdateLib.bat` — 각 라이브러리 헤더/lib을 `GameEngineLib/Inc/`, `Client/Lib/`으로 자동 복사

---

## RenderLib — 렌더링 엔진

### 초기화 흐름

```
Render::InitRender(true, sizeX, sizeY, hwnd)
  ├── RenderDevice::Initialize()      // D3D11 Device + DeviceContext 생성
  ├── Renderer::Initialize()          // SwapChain, BackBuffer RTV, DSV, Sampler 생성
  └── RenderPipeline::Initialize()    // RenderPass 배열 초기화

GameEngine::InitEngine()
  ├── GeometryManager::Initialize()
  ├── ShaderManager::Initialize()
  ├── ModelManager::Initialize()
  └── TextureManager::Initialize()
```

---

### 렌더 루프 (매 프레임)

BeginFrame → Submit(게임 로직 중) → Execute 순서가 반드시 지켜져야 합니다.

```
MainApp::Loop()
  ├── RenderPipeline::BeginFrame()           // ① 모든 Pass 큐 비우기
  ├── SceneManager::Update(dt)               // ② 게임 로직 + Submit 발생
  ├── Renderer::RenderBegin()                // RTV/DSV Clear
  ├── RenderPipeline::Execute(ctx)           // ③ 정렬 후 드로우
  └── Renderer::RenderEnd()                  // SwapChain::Present
```

---

### 6단계 정렬 우선순위

드로우콜 상태 변경 비용을 최소화하기 위해 아래 순서로 정렬합니다.

| 단계 | 내용 | API |
|:----:|------|-----|
| 1 | **Layer / Pass** — Opaque → Transparent → UI → Effect | Pass별 분리 실행 |
| 2 | **Input Layout** (Vertex 타입) | `IASetInputLayout` |
| 3 | **Render Target** (MRT) | `OMSetRenderTargets` |
| 4 | **Shader** (VS / PS) | `VSSetShader` / `PSSetShader` |
| 5 | **Texture** (SRV) | `PSSetShaderResources` |
| 6 | **Per-Object CB + Draw** | `Map/Discard` + `DrawIndexed` |

---

### 클래스 구조

```
RenderPipeline  (singleton)
  └── array<RenderPass, 4>          (Opaque / Transparent / UI / Effect)
        ├── vector<RenderCommand>   (매 프레임 Submit된 드로우 요청)
        └── RenderTargetGroup*      (nullptr = Backbuffer 사용)

RenderCommand
  ├── Geometry*   — VB / IB
  └── Material*   — ShaderGroup* + CB 슬롯들

ShaderGroup
  ├── VertexShader   — HLSL 컴파일, Blob 보관
  ├── PixelShader
  └── InputLayout    — VS Blob 리플렉션으로 자동 생성

Material  (추상 base)
  ├── Initialize(ShaderGroup*)      — CB 등록 및 슬롯 연결
  ├── BindMaterial()                — CB 업로드 + 슬롯 바인드
  ├── Clone()                       — CPU 데이터 독립 복사본 반환
  ├── virtual SetWorld(float4x4)
  └── virtual SetViewProj(float4x4)
```

Material CB 등록 시 `AutomaticRegisterVS<T>` / `AutomaticRegisterPS<T>` 헬퍼를 사용합니다.
같은 이름의 CB는 GPU 버퍼를 공유하고, 각 Material 인스턴스는 독립적인 CPU 데이터 포인터를 가집니다.

---

### 버텍스 포맷 (`RenderLib/BufferStruct.h`)

| 구조체 | 요소 | 용도 |
|--------|------|------|
| `VTXPOS` | position | 단순 위치 |
| `VTXPOSCOL` | position, color | 색상 |
| `VTXPOSTEX` | position, texcoord | 텍스처 |
| `VTXPOSNORMTANUV` | position, normal, tangent, texcoord | 스태틱 메시 |
| `VTXPOSNORMTANUVBLEND` | +blendIndex, blendWeight | 애니메이션 메시 |

---

### 싱글톤 목록

| 싱글톤 | 역할 |
|--------|------|
| `RenderDevice` | D3D11 Device / DeviceContext 보유 |
| `Renderer` | SwapChain, BackBuffer RTV/DSV, Sampler |
| `RenderPipeline` | RenderPass 배열, Submit/Execute 관리 |

---

### RenderTargetGroup (MRT)

```cpp
RenderTargetGroup rtGroup;
rtGroup.Create(device, width, height,
    { {DXGI_FORMAT_R8G8B8A8_UNORM}, {DXGI_FORMAT_R16G16B16A16_FLOAT} },
    /*hasDepth=*/true);

pass.SetRenderTarget(&rtGroup);   // nullptr = Backbuffer 사용
```

---

## GameEngineLib — 게임 엔진

RenderLib 위에서 동작하며, 씬 관리, GameObject/Component 계층, 리소스 매니저를 제공합니다.

---

### 씬 / 오브젝트 / 컴포넌트 구조

```
SceneManager  (singleton)
  └── Scene  (현재 활성 씬)
        └── vector<GameObject>
              ├── Transform      (항상 보유, 위치/회전/스케일)
              └── vector<Component>
```

**씬 전환**: `SceneManager::LoadScene<T>()` 호출 → 다음 `Update()` 시 `Awake()` → `Start()` 순으로 초기화.

**컴포넌트 라이프사이클**:
```
Awake()        ← AddComponent 직후 즉시 호출
Start()        ← 씬 로드 직후 (모든 Awake 완료 후)
Update(dt)     ← 매 프레임
LateUpdate(dt) ← 매 프레임 (모든 Update 완료 후)
OnDestroy()    ← DestroyGameObject 시
```

---

### 카메라

`CameraComponent`가 `ViewProjManager`를 대체합니다.

```cpp
auto* go = CreateGameObject("Camera");
auto* cam = go->AddComponent<CameraComponent>();
cam->SetPerspective(XM_PIDIV4, aspect, 0.1f, 1000.f);
cam->SetLookAt(eye, target, up);

// 팔로우 카메라
cam->SetFollowTarget(playerTransform, {0, 5, -10});
cam->SetSmoothSpeed(5.f);  // 0 = 하드 팔로우
```

`LateUpdate()`에서 뷰/프로젝션 행렬이 갱신되며, `RenderComponent`가 `GetViewProj()`를 참조합니다.

---

### 리소스 매니저

`GeometryManager`, `ShaderManager`, `MaterialManager`, `ModelManager`, `TextureManager` 싱글톤이 리소스를 캐시합니다.

| 싱글톤 | `Get()` 반환 | 역할 |
|--------|-------------|------|
| `GeometryManager` | `Geometry*` (공유 불변) | VB/IB 동기·비동기 로드·캐시 |
| `ShaderManager` | `ShaderGroup*` (공유 불변) | VS+PS 로드 → ShaderGroup 생성·캐시 |
| `MaterialManager` | `unique_ptr<Material>` (클론) | ShaderGroup 준비 후 Initialize, clone 반환 |
| `ModelManager` | `Model*` (공유 불변) | `.mymesh` 파일 로드·캐시 |
| `TextureManager` | `Texture*` (공유 불변) | DDS/WIC 로드·캐시 |

`MaterialManager::Get()`은 매번 Clone을 반환합니다. GPU CB 버퍼는 공유되므로 복사 비용은 CPU 데이터 포인터 교체뿐입니다.

---

## Tool — 모델 익스포터

Tool 프로젝트는 Assimp로 `.fbx` 등을 읽어 커스텀 포맷으로 저장합니다. Client는 Assimp를 사용하지 않습니다.

**커스텀 포맷**:
| 확장자 | 내용 |
|--------|------|
| `.mymesh` | 스태틱 메시 정점 데이터 (VTXPOSNORMTANUV) |
| `.myanimmesh` | 애니메이션 메시 정점 데이터 (+BlendIndex/Weight, Bone 정보) |
| `.myanim` | 애니메이션 키프레임 데이터 |
| `.mymat` | Material 정보 (Diffuse, Normal 등 텍스처 경로) |

---

## 추후 작업

### 진행 중 / 예정
- [ ] **`ConstantBufferManager` 위치 변경** — `Render::ConstantBufferManager` → `Engine::ConstantBufferManager`. Material과의 의존성 구조 개선
- [ ] **Assimp 모델 로딩** — Tool에서 `.fbx` → `.mymesh` / `.mymat` 변환 완성. Client에서 `.mymesh` 직접 로드
- [ ] **Deferred Shading** — G-Buffer(Albedo/Normal/WorldPos) + Lighting Pass 구조 전환. `RenderPass` 상속 구조(`GeometryPass`, `LightingPass`, `PostProcessPass`) 도입

### RenderLib
- [ ] Transparent Pass — Back-to-Front 깊이 역순 정렬
- [ ] G-Buffer용 `RenderTargetGroup` 연결 (지연 렌더링)

### GameEngineLib
- [ ] `Input` 시스템 — 키보드/마우스 입력 처리
- [ ] 씬 전환 시 이전 씬 리소스 해제 흐름 정리

### Client
- [ ] 여러 GameObject 씬 구성 및 Transform 계층(부모/자식) 연결
