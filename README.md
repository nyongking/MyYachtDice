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
├── CoreLib/          # 메모리, 스레드, 잡큐, 타이머 등 엔진 코어
├── ServerCoreLib/    # IOCP 네트워크 (Listener, Session, Service)
├── RenderLib/        # DirectX 11 렌더링 엔진
├── GameEngineLib/    # 게임 엔진 (씬, 오브젝트, 컴포넌트, 프레임 타이머)
└── Client/           # 게임 클라이언트 진입점 및 씬/컴포넌트 구현
```

**빌드 환경**
- Visual Studio 2022, Windows SDK, C++17
- 빌드: `msbuild Client\Client.vcxproj /p:Configuration=Debug`
- PreBuildEvent: `UpdateLib.bat` — 각 라이브러리 헤더를 `Client/Inc/`로 자동 복사

---

## RenderLib — 렌더링 엔진

### 초기화 흐름

```
// ── Client.cpp (WinMain) ──
InitCore()

Render::InitRender(true, sizeX, sizeY, hwnd)
  ├── RenderDevice::Initialize()          // D3D11 Device + DeviceContext 생성
  ├── Renderer::Initialize()              // SwapChain, BackBuffer RTV, DSV, Sampler 생성
  ├── ConstantBufferManager::Initialize() // 이름 기반 CB 공유 풀 초기화
  └── RenderPipeline::Initialize()        // RenderPass 배열 초기화 (4개)

// ── MainApp::Init() → InitGame() ──
GameEngine::InitEngine()
  ├── GeometryManager::Initialize()       // VB/IB 생성용 Device/Context 등록
  └── ShaderManager::Initialize()         // VS/PS 로드·컴파일용 Device/Context 등록
```

---

### 렌더 루프 (매 프레임)

BeginFrame → Submit(게임 로직 중) → Execute 순서가 반드시 지켜져야 합니다.

```
MainApp::Loop()
  │
  ├── RenderPipeline::BeginFrame()           // ① 모든 Pass 큐 비우기
  │
  ├── SceneManager::Update(dt)               // ② 게임 로직
  │     └── [각 Component::Update()]
  │           └── RenderPipeline::Submit(layer, cmd)   // ← Submit 발생
  │
  ├── Renderer::RenderBegin()                // RTV/DSV Clear
  │
  ├── RenderPipeline::Execute(ctx)           // ③ 정렬 후 드로우
  │     ├── RenderPass[Opaque]::Execute(ctx)
  │     ├── RenderPass[Transparent]::Execute(ctx)
  │     ├── RenderPass[UI]::Execute(ctx)
  │     └── RenderPass[Effect]::Execute(ctx)
  │
  └── Renderer::RenderEnd()                  // SwapChain::Present
```

---

### 6단계 정렬 우선순위

드로우콜 상태 변경 비용을 최소화하기 위해 아래 순서로 정렬합니다.

| 단계 | 내용 | API | 담당 클래스 |
|:----:|------|-----|-------------|
| 1 | **Layer / Pass** — Opaque → Transparent → UI → Effect | Pass별 분리 실행 | `RenderPass::Layer` |
| 2 | **Input Layout** (Vertex 타입) | `IASetInputLayout` | `ShaderGroup` |
| 3 | **Render Target** (MRT) | `OMSetRenderTargets` | `RenderTargetGroup` |
| 4 | **Shader** (VS / PS) | `VSSetShader` / `PSSetShader` | `ShaderGroup` |
| 5 | **Texture** (SRV) | `PSSetShaderResources` | `Material` |
| 6 | **Per-Object CB + Draw** | `Map/Discard` + `DrawIndexed` | `RenderCommand` |

> `RenderPass::Execute` 내부에서 ShaderGroup 포인터 기준으로 `stable_sort`하여
> 동일 셰이더 오브젝트를 묶어 단계 2·4의 바인드 횟수를 최소화합니다.

---

### 클래스 다이어그램

```
RenderPipeline  (singleton)
  └── array<RenderPass, 4>          (Opaque / Transparent / UI / Effect)
        ├── vector<RenderCommand>   (매 프레임 Submit된 드로우 요청)
        └── RenderTargetGroup*      (nullptr = Backbuffer 사용)

RenderCommand  (경량 구조체, Component가 생성)
  ├── Geometry*   — VB / IB
  └── Material*   — ShaderGroup* + CB 슬롯들

ShaderGroup
  ├── VertexShader   — HLSL 컴파일, Blob 보관, Bind(ctx)
  ├── PixelShader    — HLSL 컴파일, Bind(ctx)
  └── InputLayout    — VS Blob 반사(Reflection)로 자동 생성

Material  (추상 base)
  ├── Initialize(ShaderGroup*)       — CB 등록 및 슬롯 연결
  ├── BindMaterial()                 — CB 업로드 + 슬롯 바인드
  ├── GetShaderGroup()               — RenderPass 정렬에 사용
  ├── virtual SetWorld(float4x4)     — 월드 행렬 (no-op 기본)
  ├── virtual SetViewProj(float4x4)  — VP 행렬 (no-op 기본)
  ├── vector<ConstantBufferParameter>   — CB 수명 관리 (pData + shared_ptr<CB>)
  └── vector<ConstantBufferSlot>        — VS/PS 슬롯 바인딩

DefaultColorMaterial : Material
  ├── SetViewProj() override  → VS b0 (cbuffer "PerFrame")
  ├── SetWorld()    override  → VS b1 (cbuffer "PerObject")
  └── SetColor()              → PS b0 (cbuffer "ColorBuffer")
```

---

### Material CB 등록 패턴

```cpp
// Initialize(ShaderGroup*) 내부에서 호출

// VS 슬롯 등록: AutomaticRegister<T>(cbName, hlslCbufferName, &data, shaderGroup)
AutomaticRegister<float4x4>("PerFrame",   "PerFrame",      &m_viewProj, pShaderGroup);
AutomaticRegister<float4x4>("PerObject",  "PerObject",     &m_world,    pShaderGroup);

// PS 슬롯 등록: AutomaticRegisterPS<T>(cbName, hlslCbufferName, &data, shaderGroup)
AutomaticRegisterPS<float4>("ColorData",  "ColorBuffer",   &m_color,    pShaderGroup);

// 등록 흐름:
// 1) ConstantBufferManager::AddOrGetConstantBuffer(name, size) — 이름으로 GPU CB 공유
// 2) ConstantBufferParameter — pData 포인터 보관 (Material 멤버 변수 참조)
// 3) ConstantBufferSlot — VS/PS 슬롯 번호를 Shader Reflection으로 조회해 연결
// 4) BindMaterial() 시 pData → GPU 업로드 후 바인드
```

> **CB 공유 설계**: 같은 이름의 CB는 GPU 버퍼를 공유합니다.
> 각 Material 인스턴스는 자신만의 `pData` 포인터를 가지므로,
> `BindMaterial()` 호출 시 해당 인스턴스의 데이터를 덮어쓰고 바인딩합니다.

---

### HLSL 셰이더 (VSPOSTEX / PSPOSTEX)

```hlsl
// VSPOSTEX.hlsl
cbuffer PerFrame  : register(b0) { row_major float4x4 g_viewProj; }
cbuffer PerObject : register(b1) { row_major float4x4 g_world;    }

// PSPOSTEX.hlsl
cbuffer ColorBuffer : register(b0) { float4 g_color; }
```

> `AutomaticRegister`의 `slotname` 파라미터는 HLSL cbuffer 이름과 일치해야 합니다.
> Shader Reflection으로 해당 cbuffer의 실제 bind point(b0, b1 등)를 조회합니다.

---

### 싱글톤 목록

| 싱글톤 | 역할 |
|--------|------|
| `RenderDevice` | D3D11 Device / DeviceContext 보유 |
| `Renderer` | SwapChain, BackBuffer RTV/DSV, Sampler |
| `ConstantBufferManager` | 이름 기반 CB 공유 풀 |
| `RenderPipeline` | RenderPass 배열, Submit/Execute 관리 |
| `ViewProjManager` | 카메라 View/Projection 행렬 *(제거 예정)* |

---

### 리소스 2단계 로딩 패턴

```
워커 스레드 (파일 I/O)          메인 스레드 (GPU 생성)
──────────────────────          ──────────────────────
LoadFromFile(path)       →→→    CreateFromShaderBlob(device)
  D3DCompileFromFile              CreateVertexShader
  D3DReflect                      CreateInputLayout (ShaderGroup)
  ReflectShader()                 CreateBuffer (Geometry)
```

> Direct3D 디바이스/컨텍스트 호출은 스레드 안전하지 않으므로
> GPU 생성 단계는 반드시 메인 스레드에서 수행합니다.

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

RenderLib 위에서 동작하며, 게임 로직과 렌더링 사이를 연결하는 층입니다.
씬 관리, 게임 오브젝트/컴포넌트 계층, 프레임 타이머를 제공합니다.

---

### Timer — 프레임 타이머

QPC(QueryPerformanceCounter) 기반 고정밀 타이머.
Accumulator spin 방식으로 목표 FPS를 맞추고 정확한 델타타임을 공급합니다.

#### 동작 원리

```
MainApp::Loop() 호출 (tight spin)
  └── Timer::Tick()
        ├── rawDelta = QPC 경과 시간
        ├── sinceLastFrame += rawDelta      ← 프레임 간 dt 누적
        ├── accumulator    += rawDelta
        │
        ├── [FPS 제한 있을 때]
        │     accumulator >= targetInterval ?
        │       YES → frameReady = true
        │               deltaTime    = sinceLastFrame   ← 실제 프레임 dt
        │               sinceLastFrame = 0
        │               accumulator  -= targetInterval  (초과분 유지, 드리프트 방지)
        │       NO  → frameReady = false (조기 return)
        │
        └── FPS 측정: rawDelta 무조건 누적 / frameReady 시 frameCount++
                      1초마다 currentFPS 갱신
```

> `deltaTime`은 마지막 frame-ready 이후 누적된 실제 경과 시간입니다.
> spin 틱 단위의 미세한 raw delta를 그대로 반환하지 않습니다.

#### API

```cpp
GameEngine::Timer timer;

timer.Reset();               // 게임 시작 전 1회 호출
timer.SetTargetFPS(60);      // 0 = 무제한

// 매 루프
timer.Tick();
if (!timer.IsFrameReady())
    return;

float dt  = timer.GetDeltaTime();   // ≈ 0.0167 (60fps 기준)
float t   = timer.GetTotalTime();   // 누적 경과 시간
int   fps = timer.GetFPS();         // 측정된 실제 FPS
```

---

### 씬 / 오브젝트 / 컴포넌트 구조

#### 계층 구조

```
SceneManager  (singleton)
  └── Scene  (현재 활성 씬)
        └── vector<GameObject>
              ├── Transform      (항상 보유, 위치/회전/스케일)
              └── vector<Component>  (추가된 컴포넌트들)
```

#### 씬 전환

```cpp
// 다음 Update() 진입 시 ApplyPendingScene()으로 교체 (pending → current)
// Awake() → Start() 순서로 호출됨
SceneManager::GetInstance().LoadScene<MyScene>();
```

#### 컴포넌트 라이프사이클

```
Awake()        ← AddComponent 직후 즉시 호출
Start()        ← 씬 로드 직후 호출 (모든 Awake 완료 후)
Update(dt)     ← 매 프레임
LateUpdate(dt) ← 매 프레임 (모든 Update 완료 후)
OnDestroy()    ← DestroyGameObject 시 호출
```

#### 클래스 다이어그램

```
Component  (abstract)
  ├── virtual Awake / Start / Update / LateUpdate / OnDestroy
  └── GetOwner() : GameObject*

Transform : Component
  ├── m_position : float3
  ├── m_rotation : float3  (오일러 각, 도 단위)
  ├── m_scale    : float3
  └── GetWorldMatrix() : float4x4  (TRS 행렬, dirty flag 캐시)

GameObject
  ├── m_transform  : unique_ptr<Transform>   ← 항상 존재
  ├── m_components : vector<unique_ptr<Component>>
  ├── AddComponent<T>(args...)  → T*   (Awake 즉시 호출)
  └── GetComponent<T>()         → T*

Scene
  ├── virtual Awake() / Start()
  ├── CreateGameObject(name) → GameObject*
  ├── DestroyGameObject(go)
  ├── Update(dt)    → 모든 GO의 Update 호출
  └── LateUpdate(dt)

SceneManager  (singleton)
  ├── LoadScene<T>()     → pending 등록
  ├── Update(dt)         → ApplyPendingScene() + currentScene->Update(dt)
  └── GetCurrentScene()  → Scene*
```

---

### 타입 별칭

`RenderLib/RenderTypes.h` 에 정의됩니다.
두 별칭 모두 `DirectX::XMFLOAT*`의 alias이므로 실제 타입이 동일합니다.

```cpp

// namespace Render     (RenderTypes.h)  — 동일하게 정의
using float2   = DirectX::XMFLOAT2;
using float3   = DirectX::XMFLOAT3;
using float4   = DirectX::XMFLOAT4;
using float4x4 = DirectX::XMFLOAT4X4;
using _matrix  = DirectX::XMMATRIX;
using fmatrix  = DirectX::FXMMATRIX;
```

> GameEngineLib 헤더는 `EngineTypes.h`를 직접 포함해 self-contained를 유지합니다.
> Client PCH(`ClientPch.h`)가 `EngineTypes.h`를 포함하지 않기 때문입니다.

---

### 리소스 매니저

`GeometryManager`, `ShaderManager`, `MaterialManager` 세 싱글톤이 리소스를 캐시하며, 모두 `ResourceCache<T>`를 상속합니다. `InitEngine()`에서 Device/Context를 등록해야 동작합니다.

| 싱글톤 | `Get()` 반환 | 역할 |
|--------|-------------|------|
| `GeometryManager` | `Geometry*` (공유 불변) | VB/IB 동기·비동기 로드·캐시 |
| `ShaderManager` | `ShaderGroup*` (공유 불변) | VS+PS 로드 → ShaderGroup 생성·캐시 |
| `MaterialManager` | `unique_ptr<Material>` (클론) | ShaderGroup 준비 후 Initialize, clone 반환 |

```
ResourceState: NotLoaded → Loading → Ready / Failed

GeometryManager::LoadSync(key, geo)    // 메인 스레드에서 VB/IB 생성
GeometryManager::LoadAsync(key, geo)   // I/O는 워커, GPU 생성은 메인으로 디스패치
GeometryManager::Get(key)             // Ready 상태의 Geometry* 반환

ShaderManager::LoadSync(key, vsPath, psPath)
ShaderManager::Get(key)               // ShaderGroup* 반환

MaterialManager::LoadSync(key, mat, shaderKey)
MaterialManager::Get(key)             // Clone된 unique_ptr<Material> 반환
```

> `MaterialManager::Get()`은 매번 Clone을 반환합니다. GPU CB 버퍼는 `ConstantBufferManager`를 통해 공유되므로 복사 비용은 CPU 데이터 포인터 교체뿐입니다.

---

## Client — 통합 예제

RenderLib + GameEngineLib 를 연결하는 씬/컴포넌트 구현체입니다.

### QuadComponent

임의의 `Material`을 받아 매 프레임 Transform → Material → Submit 흐름을 수행합니다.
Material을 교체하는 것만으로 색상 전용 / 텍스처+색상 등 다양한 렌더링이 가능합니다.

```cpp
class QuadComponent : public GameEngine::Component
{
public:
    explicit QuadComponent(std::shared_ptr<Render::Material> material);
    void Update(float dt) override;
};

// Update() 내부 흐름
void QuadComponent::Update(float dt)
{
    // 1. 월드 행렬 — Transform으로부터 TRS 계산
    m_material->SetWorld(GetOwner()->GetTransform()->GetWorldMatrix());

    // 2. VP 행렬 — 현재 활성 카메라
    const Render::float4x4* pVP = ViewProjManager::GetInstance().GetCurrentViewProj();
    if (pVP) m_material->SetViewProj(*pVP);

    // 3. 드로우 커맨드 제출
    RenderCommand cmd;
    cmd.geometry = RenderDefaultRegistry::GetInstance().GetGeometry(GEO_QUAD_TEX).get();
    cmd.material  = m_material.get();
    RenderPipeline::GetInstance().Submit(RenderPass::Layer::Opaque, cmd);
}
```

### QuadColorScene

씬 로드 시 빨간 Quad 오브젝트를 생성하는 예제 씬입니다.

```cpp
class QuadColorScene : public GameEngine::Scene
{
public:
    void Awake() override;
};

void QuadColorScene::Awake()
{
    auto& reg = Render::RenderDefaultRegistry::GetInstance();

    // 빨간 쿼드 (원점)
    auto redMat = std::make_shared<Render::DefaultColorMaterial>();
    redMat->Initialize(reg.GetShaderGroup(Render::SHADER_QUAD_TEX).get());
    redMat->SetColor({ 1.f, 0.f, 0.f, 1.f });
    CreateGameObject("Quad")->AddComponent<QuadComponent>(redMat);

    // 파란 쿼드 (Y +1 오프셋)
    auto blueMat = std::make_shared<Render::DefaultColorMaterial>();
    blueMat->Initialize(reg.GetShaderGroup(Render::SHADER_QUAD_TEX).get());
    blueMat->SetColor({ 0.f, 0.5f, 1.f, 1.f });
    auto* go = CreateGameObject("Quad");
    go->AddComponent<QuadComponent>(blueMat);
    go->GetTransform()->SetPosition(GameEngine::float3(0.f, 1.f, 0.f));
}
```

### MainApp — 프레임 루프 통합

```
Client 메인 루프 (tight spin)
  └── MainApp::Loop()
        ├── Timer::Tick()
        ├── !IsFrameReady() → Sleep(1); return   ← CPU 스핀 완화
        │
        ├── dt = GetDeltaTime()               ← 마지막 프레임 이후 실제 경과 시간
        │
        ├── RenderPipeline::BeginFrame()       ← ① Submit 전에 반드시 호출
        │
        ├── SceneManager::Update(dt)           ← ② 게임 로직 + Submit 발생
        │     └── QuadComponent::Update()
        │           └── RenderPipeline::Submit(Opaque, cmd)
        │
        ├── Renderer::RenderBegin()            ← RTV/DSV Clear
        ├── RenderPipeline::Execute(ctx)       ← ③ 정렬 후 드로우
        ├── Renderer::RenderEnd()              ← SwapChain::Present
        │
        └── [Debug] SetWindowText → "FPS: 60  dt: 0.0167"
```

```cpp
// MainApp::Init() — 카메라 + 씬 로드
m_camID = ViewProjManager::GetInstance().CreateViewProj();
ViewProjManager::GetInstance().UpdateViewProjByID(m_camID, view, proj);
ViewProjManager::GetInstance().ChangeCurrent(m_camID);

SceneManager::GetInstance().LoadScene<QuadColorScene>();

timeBeginPeriod(1);    // 타이머 정밀도 1ms
m_timer.Reset();
m_timer.SetTargetFPS(144);
```

---

## 추후 작업

### 진행 중 / 예정 (Plan.md 기준)
- [ ] **Material Release 정리** — `Material::Release()` 경로 제거. `ConstantBufferManager`가 버퍼를 레퍼런스 카운트 대신 `unique_ptr`로 보유하도록 변경
- [ ] **Clone 수정** — `Clone()` 시 `ConstantBufferParameter::m_pData`가 원본이 아닌 클론의 멤버를 가리키도록 교체. `Clone()`에서 `Initialize()` 호출 제거
- [ ] **`ViewProjManager` 제거** — 뷰/프로젝션 데이터 관리 방식 교체
- [ ] **`DefaultColorMaterial`을 Client / Tool로 이동** — `RenderLib`에 있어선 안 되는 소비자 계층 코드
- [ ] **Assimp 모델 로딩** — Assimp로 메시 데이터를 로드하고 커스텀 파싱 파이프라인을 거쳐 `Geometry` 서브클래스로 변환

### RenderLib
- [ ] Transparent Pass — Back-to-Front 깊이 역순 정렬
- [ ] Texture(SRV) 기준 정렬 추가 (6단계 5번)
- [ ] G-Buffer용 `RenderTargetGroup` 연결 (지연 렌더링)

### GameEngineLib
- [ ] `Input` 시스템 — 키보드/마우스 입력 처리
- [ ] `Camera` 컴포넌트 — 뷰/프로젝션 데이터 공급 방식 설계
- [ ] 씬 전환 시 이전 씬 리소스 해제 흐름 정리

### Client
- [ ] Texture 렌더링 — TextureComponent + TextureMaterial 구현
- [ ] 여러 GameObject 씬 구성 및 Transform 계층(부모/자식) 연결
