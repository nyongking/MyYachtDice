# YachtDice

Windows C++ 클라이언트 게임 프로젝트. DirectX 11 Deferred Shading 렌더링 엔진과 자체 3D 물리 엔진을 직접 구현합니다.



## 개요

1. 게임 'Yacht Dice'를 모방한 게임을 만듭니다. (예정)
2. IOCP, TCP 서버를 이용하여 송수신을 합니다. (예정)
3. 자체 3D 리지드바디 물리 엔진(PhysicsLib)을 구현하여 게임에 활용합니다. (구현 완료)



---

## 프로젝트 구조

```
YachtDice/
├── CoreLib/          # 메모리, 스레드, 잡큐, 동기화, 병렬 처리 등 엔진 코어
├── PhysicsLib/       # 3D 리지드바디 물리 엔진 (충돌 감지/해결, Island 병렬 솔빙)
├── ServerCoreLib/    # IOCP 네트워크 (Listener, Session, Service)
├── RenderLib/        # DirectX 11 Deferred Shading 렌더링 엔진
├── GameEngineLib/    # 게임 엔진 (씬, 오브젝트, 컴포넌트, 리소스 매니저, 물리 통합)
├── Client/           # 게임 클라이언트 진입점 및 씬/컴포넌트 구현
├── Tool/             # ImGui/ImGuizmo 에디터 + Assimp FBX 익스포터
└── Docs/             # HTML 프로젝트 문서
```

**빌드 환경**
- Visual Studio 2022, Windows SDK, C++17
- 빌드 순서: CoreLib → PhysicsLib → RenderLib → GameEngineLib → Client
- 빌드: `msbuild Client\Client.vcxproj /p:Configuration=Debug /p:Platform=x64`
- PreBuildEvent: `UpdateLib.bat` — 각 라이브러리 헤더/lib을 `GameEngineLib/Inc/`, `Client/Lib/`으로 자동 복사

---

## RenderLib — Deferred Shading 렌더링 엔진

### 초기화 흐름

```
Render::InitRender(true, sizeX, sizeY, hwnd)
  ├── RenderDevice::Initialize()      // D3D11 Device + DeviceContext 생성
  ├── Renderer::Initialize()          // SwapChain, BackBuffer RTV, DSV, Sampler 생성
  └── RenderPipeline::Initialize()    // G-Buffer MRT 생성, GeometryPass / LightingPass 초기화

GameEngine::InitEngine()
  ├── GeometryManager::Initialize()
  ├── ShaderManager::Initialize()
  ├── MaterialManager::Initialize()
  ├── ModelManager::Initialize()
  ├── TextureManager::Initialize()
  ├── PhysicsManager::Initialize()
  └── ComponentRegistry: Camera / Light / Model / Quad / RigidBody Component 등록
```

---

### 렌더 루프 (매 프레임)

```
MainApp::Loop()
  ├── RenderPipeline::BeginFrame()           // ① G-Buffer 클리어
  ├── SceneManager::Update(dt)               // ② 게임 로직
  │     ├── RenderComponent::Update(dt)      //    → RenderPipeline::Submit(layer, cmd)
  │     ├── LightComponent::LateUpdate(dt)   //    → RenderPipeline::SubmitLight(cmd)
  │     └── CameraComponent::LateUpdate(dt)  //    → ViewProj 갱신
  ├── Renderer::RenderBegin()                // 백버퍼 RTV/DSV Clear
  ├── RenderPipeline::Execute(ctx)           // ③ 멀티패스 드로우 (아래 참고)
  ├── Renderer::RenderEnd()                  // SwapChain::Present
  └── RenderPipeline::EndFrame()             // 모든 커맨드 큐 클리어
```

---

### Deferred Shading Execute 순서

```
RenderPipeline::Execute(ctx)
  ├── GeometryPass[Opaque]    → G-Buffer 3 MRT (Albedo, Normal, WorldPos)
  ├── LightingPass            → G-Buffer SRV 읽기, 풀스크린 삼각형, 백버퍼에 라이팅 결과 출력
  ├── GeometryPass[Transparent] → 백버퍼 Forward 렌더링
  ├── GeometryPass[UI]          → 백버퍼
  └── GeometryPass[Effect]      → 백버퍼
```

**G-Buffer 구성 (3 MRT)**

| 슬롯 | 포맷 | 내용 |
|:----:|------|------|
| 0 | RGBA8 | Albedo(RGB) + Metallic(A) |
| 1 | RGBA16F | Normal(RGB) + Roughness(A) |
| 2 | RGBA32F | World Position(RGB) |

---

### 클래스 구조

```
RenderPipeline  (singleton)
  ├── GeometryPass[4]          (Opaque / Transparent / UI / Effect)
  │     ├── vector<RenderCommand>   (매 프레임 Submit된 드로우 요청)
  │     ├── RenderTargetGroup*      (Opaque = G-Buffer, 나머지 = 백버퍼)
  │     └── ConstantBuffer          (cbPerFrame b0: ViewProj, cbPerObject b1: World)
  └── LightingPass
        ├── vector<LightCommand>    (매 프레임 SubmitLight된 라이트 요청)
        ├── RenderTargetGroup*      (G-Buffer, SRV로 읽기)
        └── Material*               (DeferredLightingMaterial)

RenderCommand  (RenderItem.h)
  ├── Geometry*    — VB / IB
  ├── Material*    — ShaderGroup* + CB / Texture 슬롯
  ├── float4x4 world
  └── float4x4 viewProj

LightCommand  (LightItem.h)
  └── LightData    — type / color / position / range / direction / intensity / spotAngles

RenderPassBase  (추상)
  ├── GeometryPass  — RenderCommand 큐, ShaderGroup 기준 stable_sort
  └── LightingPass  — LightCommand 큐, 풀스크린 삼각형

ShaderGroup
  ├── VertexShader   — HLSL 컴파일, Blob 보관
  ├── PixelShader
  └── InputLayout    — VS Blob 리플렉션으로 자동 생성

Material  (추상 base)
  ├── Initialize(ShaderGroup*, device)  — CB / Texture 슬롯 등록
  ├── BindMaterial(ctx)                 — CB 업로드 + 슬롯 바인드 + 텍스처 바인드
  ├── Clone()                           — CPU 데이터 독립 복사본 반환
  ├── SetParam<T>(name, value)          — CB 파라미터 이름으로 값 쓰기
  ├── SetTexture(name, tex/srv)         — 텍스처 슬롯 업데이트
  └── virtual SetLights(lights, count)  — Lighting Material에서 override
```

> **주의**: `Material`은 `SetWorld()` / `SetViewProj()`를 갖지 않습니다. 행렬은 `RenderCommand.world / viewProj`에 직접 포함되며, `GeometryPass`가 내부 CB에 업데이트합니다.

Material CB 등록 시 `AutomaticRegisterVS<T>` / `AutomaticRegisterPS<T>` 헬퍼를 사용합니다.

---

### G-Buffer / Lighting HLSL 셰이더 (`Bin/Shader/`)

| 파일 | 역할 |
|------|------|
| `VSGBUFFER.hlsl` | G-Buffer 패스 VS — 위치/법선/탄젠트/UV 변환 |
| `PSGBUFFER.hlsl` | G-Buffer 패스 PS — Albedo / Normal / WorldPos MRT 출력 |
| `VSFULLSCREEN.hlsl` | Lighting 패스 VS — SV_VertexID 풀스크린 삼각형 (VB 없음) |
| `PSLIGHTING.hlsl` | Lighting 패스 PS — G-Buffer SRV 읽기 → 최종 색상 출력 |

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
| `RenderPipeline` | GeometryPass / LightingPass 관리, Submit/Execute |

---

### RenderTargetGroup (MRT)

```cpp
RenderTargetGroup rtGroup;
rtGroup.Create(device, width, height,
    {
        {DXGI_FORMAT_R8G8B8A8_UNORM},        // Albedo + Metallic
        {DXGI_FORMAT_R16G16B16A16_FLOAT},     // Normal + Roughness
        {DXGI_FORMAT_R32G32B32A32_FLOAT},     // WorldPosition
    },
    /*hasDepth=*/true);

geometryPass.SetRenderTarget(&rtGroup);   // nullptr = 백버퍼 사용
```

---

## GameEngineLib — 게임 엔진

RenderLib 위에서 동작하며, 씬 관리, GameObject/Component 계층, 리소스 매니저를 제공합니다.

---

### 씬 / 오브젝트 / 컴포넌트 구조

```
SceneManager  (singleton)
  └── Scene  (현재 활성 씬, JSON 직렬화 지원)
        └── vector<GameObject>
              ├── Transform      (항상 보유, 위치/회전/스케일)
              └── vector<Component>
                    ├── RenderComponent  (→ ModelComponent / QuadComponent)
                    ├── CameraComponent
                    ├── LightComponent
                    └── RigidBodyComponent  (물리 시뮬레이션)
```

**씬 전환**: `SceneManager::LoadScene<T>()` 호출 → 다음 `Update()` 시 `Awake()` → `Start()` 순으로 초기화.

**씬 직렬화**: `Scene::SaveToFile(path)` / `LoadFromFile(path)` — JSON 포맷. `ComponentRegistry`가 타입명으로 Component 인스턴스를 생성합니다.

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

```cpp
auto* go  = scene->CreateGameObject("Camera");
auto* cam = go->AddComponent<CameraComponent>();
cam->SetPerspective(XM_PIDIV4, aspect, 0.1f, 1000.f);
cam->SetLookAt(eye, target, up);

// 팔로우 카메라
cam->SetFollowTarget(playerTransform, {0, 5, -10});
cam->SetSmoothSpeed(5.f);  // 0 = 하드 팔로우
```

`LateUpdate()`에서 뷰/프로젝션 행렬이 갱신되며, `RenderComponent`가 `SceneManager::GetMainCamera()`를 통해 ViewProj를 참조합니다.

---

### 라이트

```cpp
auto* go    = scene->CreateGameObject("DirectionalLight");
auto* light = go->AddComponent<LightComponent>();
light->SetType(LightType::Directional);
light->SetColor({1.f, 1.f, 1.f});
light->SetDirection({0.f, -1.f, 1.f});
light->SetIntensity(1.5f);
```

`LateUpdate()`에서 `RenderPipeline::SubmitLight()`를 호출합니다. `LightingPass`가 이를 수집해 `DeferredLightingMaterial`에 전달합니다.

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

### ComponentRegistry (씬 직렬화용 팩토리)

```cpp
// InitEngine() 내부에서 등록
ComponentRegistry::Register<CameraComponent>("CameraComponent");
ComponentRegistry::Register<LightComponent>("LightComponent");
ComponentRegistry::Register<ModelComponent>("ModelComponent");
ComponentRegistry::Register<QuadComponent>("QuadComponent");
ComponentRegistry::Register<RigidBodyComponent>("RigidBodyComponent");

// 역직렬화 시
auto comp = ComponentRegistry::Create("ModelComponent"); // unique_ptr<Component>
```

---

## PhysicsLib — 3D 리지드바디 물리 엔진

외부 의존성 없는 C++17 물리 엔진. 헤더 온리 수학 타입(Vec3, Quat, Mat3)과 충돌 감지/해결 파이프라인을 제공합니다.

### 시뮬레이션 파이프라인

```
PhysicsWorld::Step(dt)
  ├── 1. IntegrateVelocity   — 중력 적용 + 속도 적분 (Semi-implicit Euler)
  ├── 2. BroadPhase           — AABB 브루트포스 O(n²) + 레이어 마스크 필터링
  │      NarrowPhase          — 타입 디스패치 (Sphere < Box < Plane), 5개 충돌 함수
  ├── 3. IslandBuilder        — Union-Find (경로 압축 + 랭크 최적화)
  │      ContactSolver        — Sequential Impulse + Baumgarte 안정화 (β=0.2)
  ├── 4. IntegratePosition    — 위치/회전 적분, 속도 클램핑
  └── 5. ClearAccumulators    — 힘/토크 초기화
```

### 주요 클래스

| 클래스 | 역할 |
|--------|------|
| `Vec3`, `Quat`, `Mat3` | 헤더 온리 수학 타입 (union 기반) |
| `Operation.h` | 인라인 수학 함수 모음 (Dot, Cross, Normalize, Slerp 등) |
| `RigidBody` | Static/Kinematic/Dynamic. 질량, 관성 텐서, 댐핑, 힘/토크 누적 |
| `Collider` | Sphere/Box/Plane 콜라이더. 트리거, 레이어 마스크, AABB |
| `BroadPhase` | AABB 오버랩 + 레이어 필터링으로 후보 쌍 생성 |
| `NarrowPhase` | 타입별 충돌 검사 디스패치 (트리거/접촉 분리) |
| `Collision` | 5개 충돌 함수: SphereSphere, SpherePlane, SphereBox, BoxBox(SAT 15축), BoxPlane |
| `ContactSolver` | Sequential Impulse, 누적 충격량 클램핑, Coulomb 마찰 |
| `IslandBuilder` | Union-Find로 접촉 그래프에서 독립 섬 구축 → 병렬 솔빙 |
| `PhysicsWorld` | 5단계 시뮬레이션 루프, Body/Collider 관리, IJobDispatcher 병렬 처리 |

### GameEngineLib 통합

| 클래스 | 역할 |
|--------|------|
| `PhysicsManager` | PhysicsWorld 래퍼 싱글톤. 고정 타임스텝 60Hz 누적기 |
| `RigidBodyComponent` | Transform ↔ RigidBody 동기화. 콜라이더 관리, 관성 텐서 자동 계산, JSON 직렬화, 디버그 와이어프레임 |
| `PhysicsConvert` | Vec3 ↔ float3, Euler ↔ Quat 변환 (DirectXMath 경유) |
| `DebugGeometry` | WireBox, WireSphere, WirePlane 와이어프레임 지오메트리 |
| `WireframeMaterial` | 디버그 렌더링용 미니멀 머티리얼 |

```cpp
// RigidBody 사용 예시
auto* go = scene->CreateGameObject("PhysicsBox");
auto* rb = go->AddComponent<RigidBodyComponent>();
rb->SetBodyType(BodyType::Dynamic);
rb->SetMass(1.0f);
rb->SetColliderType(ColliderType::Box);
rb->SetColliderHalfExtents({0.5f, 0.5f, 0.5f});
```

---

## Tool — 에디터 / 모델 익스포터

Tool 프로젝트는 ImGui + ImGuizmo 기반 에디터와 Assimp FBX 익스포터를 포함합니다. Client는 Assimp를 사용하지 않습니다.

**에디터 기능**:
- `EditorScene`: GameObject 씬 편집, Inspector, 씬 저장/로드
- `ModelLoadScene`: FBX 로드 및 `.mymesh` 변환 확인

**커스텀 포맷**:
| 확장자 | 내용 |
|--------|------|
| `.mymesh` | 스태틱 메시 정점 데이터 (VTXPOSNORMTANUV + uint32 인덱스, 섹션별 material key 포함) |
| `.myanimmesh` | 애니메이션 메시 정점 데이터 (+BlendIndex/Weight, Bone 정보) |
| `.myanim` | 애니메이션 키프레임 데이터 |
| `.mymat` | Material 정보 (Diffuse, Normal 등 텍스처 경로) |

---

## 완료된 작업

- [x] **Deferred Shading** — G-Buffer(3 MRT: Albedo/Normal/WorldPos) + LightingPass 구현
- [x] **Assimp 모델 로딩** — Tool FBX → `.mymesh` 변환. `ModelComponent`로 Client 로드
- [x] **라이트 시스템** — `LightComponent`, Directional/Point/Spot, `DeferredLightingMaterial`
- [x] **씬 직렬화** — JSON Scene/GameObject/Component 저장·로드, `ComponentRegistry` 팩토리
- [x] **Tool 에디터** — ImGui/ImGuizmo Inspector, EditorScene
- [x] **물리 엔진 (PhysicsLib)** — 3D 리지드바디: 충돌 감지(BroadPhase AABB + NarrowPhase SAT), Sequential Impulse 솔버, Island 병렬 솔빙, Baumgarte 안정화
- [x] **물리 엔진 통합** — `PhysicsManager`(60Hz 고정 타임스텝), `RigidBodyComponent`(Transform 동기화, 콜라이더, JSON 직렬화, 디버그 와이어프레임)
- [x] **CoreLib 병렬 처리** — `ParallelFor`, `TaskGraph`, `Future`, `CountdownEvent`, `EventSignal`, `SpinWait`, `ThreadSafeHashMap`

## 추후 작업

### RenderLib
- [ ] Transparent Pass — Back-to-Front 깊이 역순 정렬
- [ ] PostProcess Pass — Bloom, Tone mapping 등

### GameEngineLib
- [ ] `Input` 시스템 — 키보드/마우스 입력 처리
- [ ] Transform 계층 — 부모/자식 관계

### Client
- [ ] 게임 씬 구성 (Yacht Dice 게임플레이)
- [ ] IOCP 서버 연동
