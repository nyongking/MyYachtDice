# CLAUDE.md

이 파일은 Claude Code(claude.ai/code)가 이 저장소에서 작업할 때 참고하는 가이드다.

## 빌드

```bash
# Client 빌드 (Debug x64) — 의존 라이브러리도 순서대로 함께 빌드됨
msbuild Client\Client.vcxproj /p:Configuration=Debug /p:Platform=x64

# 특정 라이브러리만 빌드
msbuild RenderLib\RenderLib.vcxproj /p:Configuration=Debug /p:Platform=x64
msbuild GameEngineLib\GameEngineLib.vcxproj /p:Configuration=Debug /p:Platform=x64
```

**솔루션이 강제하는 컴파일 순서**: CoreLib → RenderLib → GameEngineLib → Client. ServerCoreLib은 독립적이며 현재 Client에 링크되지 않는다.

**프리빌드 이벤트**: 각 프로젝트의 `UpdateLib.bat`이 `.lib` 파일을 `Client/Lib/`으로, 헤더를 `GameEngineLib/Inc/`로 복사한다. 저장소 경로에 한글(`바탕 화면`)이 포함되어 있어 일부 셸에서 `xcopy`가 실패할 수 있다 — 이 경우 bash `cp` 명령으로 직접 복사한다.

- `GameEngineLib/Inc/` — CoreLib·RenderLib 헤더 복사본 보관. **원본 수정 금지, 각 라이브러리 원본을 편집할 것.**
- `Client/Inc/` — assimp 헤더만 포함. 엔진 헤더 복사본 없음.

자동화 테스트는 없다.

---

## 아키텍처

커스텀 DirectX 11 렌더링 엔진을 포함하는 4계층 C++ 게임 프로젝트다.

| 라이브러리 | 네임스페이스 | 역할 |
|-----------|-------------|------|
| `CoreLib` | `Core` | 메모리 풀, 스레드 관리, 잡 큐, 동기화 프리미티브 |
| `RenderLib` | `Render` | DirectX 11 렌더링 파이프라인 |
| `GameEngineLib` | `GameEngine` | Scene/GameObject/Component 프레임워크, 리소스 매니저, QPC 타이머 |
| `Client` | — | 게임 진입점, 구체적인 Scene·Component 구현체 |

**타입 별칭** (`RenderLib/RenderTypes.h`): `float2/3/4/float4x4` = `DirectX::XMFLOAT*`, `_vector/_matrix` = `XMVECTOR/XMMATRIX`, `RefCom<T>` = `Microsoft::WRL::ComPtr<T>`. GameEngineLib PCH에서 이 헤더를 포함하므로 양 라이브러리 모두에서 사용 가능하다.

---

### CoreLib

다른 프로젝트 라이브러리에 의존하지 않는 저수준 인프라다.

| 모듈 | 주요 클래스 |
|------|-------------|
| 메모리 | `Allocator`, `MemoryPool`, `ObjectPool` |
| 스레딩 | `ThreadManager`, `JobQueue`, `ConcurrentJobQueue`, `MainThreadQueue` |
| 동기화 | `Lock` (RW 락), `DeadLockProfiler`, `CoreTLS` |
| 컨테이너 | `LockQueue`, `Container`, `JobQueueContainer` |
| 유틸리티 | `StringUtil`, `JsonUtil`, `ConsoleUtil`, `BufferReader`, `BufferWriter` |

`MainThreadQueue`는 워커 스레드에서 디스패치된 지연 작업(GPU 리소스 생성 등)을 메인 스레드에서 소화하기 위해 `MainApp::Loop()`가 사용한다.

---

### RenderLib

DirectX 11 렌더링 파이프라인. `ID3D11DeviceContext` 호출은 thread-safe하지 않으므로 드로우 및 리소스 바인딩은 메인 스레드에서만 수행한다.

**싱글톤**: `RenderDevice`, `Renderer`, `RenderPipeline`. 모두 `GetInstance()`로 접근.

#### 프레임 순서 (엄격히 준수)

```
RenderPipeline::BeginFrame()      ← 모든 패스 큐 클리어
SceneManager::Update(dt)          ← 게임 로직; Component들이 Submit() 호출
Renderer::RenderBegin()           ← RTV/DSV 클리어
RenderPipeline::Execute(ctx)      ← 정렬 후 드로우
Renderer::RenderEnd()             ← SwapChain::Present
RenderPipeline::EndFrame()
```

`Submit()`은 `BeginFrame` → `Execute` 구간 밖에서 절대 호출하면 안 된다.

#### 주요 클래스

- **`RenderDevice`** — D3D11 Device/DeviceContext 래퍼. `GetDevice()` / `GetContext()`.
- **`Renderer`** — SwapChain, 백버퍼 RTV, DSV, 샘플러 보유. `RenderBegin()` / `RenderEnd()`.
- **`RenderPipeline`** — Layer별 `RenderPass` 배열 관리. `Submit(layer, cmd)` / `Execute(ctx)`.
- **`RenderPass`** — 레이어별 커맨드 큐. `Layer`: `Opaque(0)`, `Transparent(1)`, `UI(2)`, `Effect(3)`. `Execute()`는 ShaderGroup 포인터 기준 `stable_sort` 후 드로우.
- **`RenderTargetGroup`** — MRT 추상화. `nullptr`이면 백버퍼 사용.
- **`Geometry`** (추상) — `CreateVB<VertexT>()` / `CreateIB<IndexT>()` 헬퍼로 버퍼 생성. `BindAndDraw(ctx)`.
- **`ShaderGroup`** — VS + PS + InputLayout 묶음. VS 블롭으로 InputLayout 자동 생성.
- **`Texture`** — `LoadFromFile(device, path)`로 DDS/WIC 로드. `SRV()` 반환.

#### 버텍스 포맷 (`RenderLib/BufferStruct.h`, namespace `Render`)

| 구조체 | 요소 | 용도 |
|--------|------|------|
| `VTXPOS` | position | 단순 위치 |
| `VTXPOSCOL` | position, color | 색상 |
| `VTXPOSTEX` | position, texcoord | 텍스처 |
| `VTXPOSNORMTANUV` | position, normal, tangent, texcoord | 스태틱 메시 |
| `VTXPOSNORMTANUVBLEND` | +blendIndex, blendWeight | 애니메이션 메시 |

각 구조체는 정적 `sElements[]`(`D3D11_INPUT_ELEMENT_DESC`)를 노출하며, `ShaderGroup::Initialize()`가 이를 사용해 InputLayout 생성.

#### 머티리얼 시스템

- **`Material`** (추상) — `Initialize(ShaderGroup*)`에서 CB 등록. `BindMaterial(ctx)`. `Clone()`. `SetWorld()` / `SetViewProj()` virtual (no-op 기본).
  - CB 등록 헬퍼: `AutomaticRegisterVS<T>(cbName, hlslName, &data, sg)` / `AutomaticRegisterPS<T>`.
  - `cbName` = GPU 버퍼 공유 키. `hlslName` = HLSL cbuffer 이름 (셰이더 리플렉션으로 바인드 포인트 조회).
- **`ConstantBuffer`** — D3D11 CB 래퍼. `ConstantBufferParameter`(CPU 데이터 + CB 수명)와 `ConstantBufferSlot`(바인드 포인트)으로 분리 관리.

#### 셰이더 시스템

- **`ShaderGroup`** — VS + PS + InputLayout. `BindShaderAndLayout(ctx)`. `VSConstantBufferSlot(name, size)` / `PSConstantBufferSlot(name, size)`.
- **`VertexShader`** / **`PixelShader`** — `LoadFromFile()`로 HLSL 로드·컴파일·리플렉션.

---

### GameEngineLib

RenderLib 위에 구축된 Scene/GameObject/Component 프레임워크. 리소스 매니저 싱글톤을 소유한다.

**싱글톤**: `SceneManager`, `GeometryManager`, `ShaderManager`, `MaterialManager`, `ModelManager`, `TextureManager`.

#### 초기화

리소스 로딩 전에 반드시 `InitEngine()`(`EngineGlobal.cpp`)을 호출해야 한다.

#### Scene / GameObject / Component

- **`SceneManager`** — `LoadScene<T>()`로 씬 등록. 다음 `Update()` 시작 시 전환 (지연).
- **`Scene`** (추상) — `Awake()` → `Start()` → `Update(dt)` / `LateUpdate(dt)`. 지연 소멸 지원.
- **`GameObject`** — `vector<unique_ptr<Component>>`(수명) + `unordered_map<type_index, Component*>`(O(1) 조회) 이중 구조. `AddComponent<T>()` 시 즉시 `Awake()`. `GetTransform()` 직접 포인터 반환.
- **`Transform`** — 위치/회전(오일러 각도)/스케일. `GetWorldMatrix()` dirty flag 기반 지연 계산.
- **`Component`** (추상) — 라이프사이클: `Awake()` → `Start()` → `Update(dt)` → `LateUpdate(dt)` → `OnDestroy()`.

#### 렌더링 컴포넌트

- **`RenderComponent`** (Component 상속) — `unique_ptr<Render::Material>`, `Render::Geometry*`, Layer 보유. `Update(dt)`에서 `SetWorld()` / `SetViewProj()` 후 `Submit()`.
- **`QuadComponent`** (RenderComponent 상속) — `Awake()`에서 GeometryManager로부터 `"Quad"` 지오메트리 조회.
- **`Quad`** (Render::Geometry 상속) — `VTXPOSTEX` 버텍스, `uint16_t` 인덱스 6개(CCW 삼각형 2개).

#### 카메라

- **`CameraComponent`** (Component 상속) — `SetPerspective()` / `SetOrthographic()`. 팔로우 카메라 지원(`SetFollowTarget()`, `SetSmoothSpeed()`). `LateUpdate()`에서 뷰/프로젝션 갱신. `GetViewProj()` / `GetView()` / `GetProj()` / `GetInverseView()`.

#### 리소스 매니저

모두 `ResourceCache<T>` 상속 싱글톤. `Lock`으로 스레드 안전.

| 싱글톤 | `Get()` 반환 | 역할 |
|--------|-------------|------|
| `GeometryManager` | `Geometry*` (공유 불변) | VB/IB 동기·비동기 로드·캐시 |
| `ShaderManager` | `ShaderGroup*` (공유 불변) | VS+PS 로드 → ShaderGroup 생성·캐시 |
| `MaterialManager` | `unique_ptr<Material>` (클론) | ShaderGroup 준비 후 Initialize, clone 반환 |
| `ModelManager` | `Model*` (공유 불변) | `.mymesh` 파일 로드·캐시 |
| `TextureManager` | `Texture*` (공유 불변) | DDS/WIC 로드·캐시 |

---

### Client / Tool

- **Client** — 게임 클라이언트. `MainApp`, 씬/컴포넌트 구현체. Assimp 미사용.
- **Tool** — 에디터/익스포터. Assimp로 `.fbx` 등을 읽어 커스텀 포맷(`.mymesh`, `.mymat`)으로 저장. `ModelExporter`, `ModelLoadScene` 등 보유.

소비자 계층 Material(`ColorMaterial`, `DiceMaterial`, `ModelMaterial`, `QuadTexMaterial` 등)은 Client 또는 Tool에 위치한다.

---

## 새로운 렌더 가능 컴포넌트 추가 방법

1. `Material` 서브클래스 작성 (Client 또는 Tool). `Initialize(ShaderGroup*)`에서 `AutomaticRegisterVS` / `AutomaticRegisterPS`로 CB 등록.
2. 새 셰이더 쌍이 필요하면 `ShaderManager::LoadSync(key, vsPath, psPath)`로 로드.
3. `RenderComponent` 상속 컴포넌트 작성. `Awake()`에서 지오메트리(`GeometryManager::Get()`)와 머티리얼(`MaterialManager::Get()`) 설정. 제출은 `RenderComponent::Update(dt)` 자동 처리.
4. `Scene::Awake()`에서 GameObject에 컴포넌트 부착.

---

## 예정 작업

`Plan.md` 기준:

- **`ConstantBufferManager` 위치 변경** — `Render::ConstantBufferManager` → `Engine::ConstantBufferManager`로 이동. `Material`과의 의존성 구조 개선.
- **Assimp 모델 로딩** — Tool에서 `.fbx` → `.mymesh` / `.mymat` 변환 파이프라인 완성. Client에서 `.mymesh` 직접 로드.
- **Deferred Shading** — G-Buffer(MRT) + Lighting Pass 구조로 전환. `RenderPass` 상속 구조(`GeometryPass`, `LightingPass`, `PostProcessPass`) 도입.
