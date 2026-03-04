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

**프리빌드 이벤트**: 각 프로젝트의 `UpdateLib.bat`이 `.lib` 파일을 `Client/Lib/`으로, 헤더를 `GameEngineLib/Inc/`로 복사한다. 저장소 경로에 한글(`바탕 화면`)이 포함되어 있어 일부 셸에서 `xcopy`가 실패할 수 있다 — 이 경우 bash `cp` 명령으로 직접 복사한다. `Client/UpdateLib.bat` 15번 줄에 알려진 오타: `zxcopy` → `xcopy`.

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

**타입 별칭** (`RenderLib/RenderTypes.h`, 네임스페이스 없는 전역): `float2/3/4/float4x4` = `DirectX::XMFLOAT*`, `_vector/_matrix` = `XMVECTOR/XMMATRIX`, `RefCom<T>` = `Microsoft::WRL::ComPtr<T>`. GameEngineLib의 PCH에서 이 헤더를 포함하므로 양 라이브러리 모두에서 사용 가능하다.

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
| 타이머 | `JobTimer` |

`MainThreadQueue`는 워커 스레드에서 디스패치된 지연 작업(GPU 리소스 생성 등)을 메인 스레드에서 소화하기 위해 `MainApp::Loop()`가 사용한다.

---

### RenderLib

DirectX 11 렌더링 파이프라인. GPU 리소스 생성(`CreateVertexShader`, `CreateInputLayout`, `CreateBuffer` 등)은 **반드시 메인 스레드에서** 이루어져야 한다. 셰이더 파일 I/O는 워커 스레드 사용 가능.

**싱글톤**: `RenderDevice`, `Renderer`, `RenderPipeline`, `ConstantBufferManager`, `ViewProjManager`. 모두 `GetInstance()`로 접근.

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

#### 디바이스 & 렌더러

- **`RenderDevice`** — D3D11 디바이스/컨텍스트 래퍼. `GetDevice()` / `GetContext()`가 모든 리소스 생성의 기본 접근자. `Renderer`, `ConstantBufferManager`의 friend.
- **`Renderer`** — SwapChain, 백버퍼 RTV, DSV, 샘플러 스테이트를 소유. `RenderBegin()`이 렌더 타겟을 클리어, `RenderEnd()`가 `Present()` 호출.

#### 파이프라인 & 패스

- **`RenderPipeline`** — `std::array<unique_ptr<RenderPass>, 4>`(Layer당 1개)를 보유하는 싱글톤. `Submit(layer, RenderCommand)`이 해당 패스 큐에 추가.
- **`RenderPass`** — 레이어별 커맨드 큐. `Layer` 열거형: `Opaque(0)`, `Transparent(1)`, `UI(2)`, `Effect(3)`. `Execute()`는 `ShaderGroup*` 포인터 기준 6단계 `stable_sort` 후 드로우:
  1. 레이어 (패스로 이미 분리됨)
  2. 인풋 레이아웃
  3. 렌더 타겟 (MRT)
  4. 셰이더 (VS/PS)
  5. 텍스처 (SRV)
  6. 오브젝트별 CB + DrawIndexed
- **`RenderCommand`** (`RenderItem.h`) — 경량 제출 단위: `Geometry* geometry` + `Material* material`.
- **`RenderTargetGroup`** — MRT 추상화. `Create()`로 텍스처/RTV/SRV 할당, `Bind()`로 설정, `GetSRV(i)`로 후속 패스에 노출. 패스의 렌더 타겟이 `nullptr`이면 백버퍼 사용.

#### 지오메트리

- **`Geometry`** (추상) — 메시 베이스 클래스. 서브클래스가 `DefaultCreateBuffers(device)` 구현. `BindAndDraw(ctx)`가 VB/IB 바인딩 후 `DrawIndexed` 발행.
  - `CreateVB<VertexT>(device, vertices)` — `m_vertexStride = sizeof(VertexT)` 자동 설정.
  - `CreateIB<IndexT>(device, indices)` — `uint16_t` / `uint32_t` 템플릿; `DXGI_FORMAT_R16_UINT` 또는 `R32_UINT` 자동 선택.

#### 머티리얼 시스템

- **`Material`** (추상) — `Initialize(ShaderGroup*)`에서 CB 등록. `BindMaterial(ctx)`가 CPU 데이터 업로드 및 슬롯 바인딩. `Clone()`이 독립적인 CPU 데이터를 가진 `unique_ptr` 사본 반환. `SetWorld()` / `SetViewProj()`는 빈 virtual (서브클래스 오버라이드용).

  `Initialize` 안에서 호출하는 CB 등록 헬퍼:
  ```cpp
  // cbName   → ConstantBufferManager의 키 (GPU 버퍼 공유 단위)
  // slotname → HLSL cbuffer 이름 (셰이더 리플렉션으로 바인드 포인트를 찾을 때 사용)
  AutomaticRegisterVS<float4x4>("PerFrame",  "PerFrame",     &m_viewProj, sg); // VS b0
  AutomaticRegisterVS<float4x4>("PerObject", "PerObject",    &m_world,    sg); // VS b1
  AutomaticRegisterPS<float4>  ("ColorData", "ColorBuffer",  &m_color,    sg); // PS b0
  ```

  GPU 버퍼는 `cbName`을 키로 `ConstantBufferManager`가 모든 머티리얼 간에 공유한다. 각 `Material` 인스턴스는 자신만의 CPU 데이터 포인터를 보유하며, `BindMaterial()`이 해당 데이터를 업로드하고 슬롯을 설정한다.

- **`DefaultColorMaterial`** — 구체 머티리얼: VS b0(`PerFrame`→ViewProj), VS b1(`PerObject`→World), PS b0(`ColorBuffer`→Color). `SetWorld()` / `SetViewProj()` / `SetColor()` 오버라이드.

- **`ConstantBufferManager`** — 싱글톤. 이름 기반 CB 캐시. `AddOrGetConstantBuffer(name, byteWidth, pInitialData, isDynamic)`으로 생성 또는 조회. `RefCountBlock`으로 레퍼런스 카운팅.

- **`ConstantBufferParameter`** (`MaterialParameter.h`) — `void*` CPU 데이터 포인터 + `shared_ptr<ConstantBuffer>`. `Update(ctx)`가 Map/Unmap으로 GPU에 업로드.

- **`ConstantBufferSlot`** (`MaterialSlot.h`) — 조회된 VS/PS 바인드 슬롯 인덱스 보유. `ConnectVSSlot(name, sg)` / `ConnectPSSlot(name, sg)`로 ShaderGroup 리플렉션에서 슬롯 조회. `BindBufferToSlot(ctx)` 호출 시 `ConstantBuffer::BindVS/BindPS` 실행.

#### 셰이더 시스템

- **`ShaderGroup`** — VS + PS + 자동 리플렉션된 `InputLayout` 묶음. `Initialize(vs, ps, device)`가 VS 블롭으로 IL 생성. `BindShaderAndLayout(ctx)`이 세 가지를 원자적으로 설정. `VSConstantBufferSlot(name, size)` / `PSConstantBufferSlot(name, size)`으로 바인드 포인트 조회.
- **`VertexShader`** / **`PixelShader`** — `LoadFromFile()`로 HLSL 로드·컴파일·리플렉션. `Shader.cpp`의 `ConstantBufferSlot()`은 수정 완료: `m_cbuffers`(이름 검증)와 `m_resources[CBUFFER]`(실제 바인드 포인트)를 교차 참조 — 기존 코드는 항상 슬롯 0 반환 버그가 있었다.
- **`ViewProjManager`** — 싱글톤. 복수의 카메라(ViewProjMatrix)를 ID로 관리. `CreateViewProj()`, `ChangeCurrent(id)`, `GetCurrentViewProj()` 등. (→ 제거 예정, 아래 예정 작업 참조)

---

### GameEngineLib

RenderLib 위에 구축된 Scene/GameObject/Component 프레임워크. 지오메트리·셰이더·머티리얼용 리소스 매니저 싱글톤을 소유한다.

**싱글톤**: `SceneManager`, `GeometryManager`, `ShaderManager`, `MaterialManager`.

#### 초기화

리소스 로딩 전에 반드시 `InitEngine()`(`EngineGlobal.cpp`)을 호출해야 한다:
```cpp
// EngineGlobal.cpp 내부
GeometryManager::GetInstance().Initialize(device, context); // RenderDevice 접근자 사용
ShaderManager::GetInstance().Initialize(device, context);
```

#### Scene / GameObject / Component

- **`SceneManager`** — `LoadScene<T>()`가 펜딩 씬을 설정하며, 다음 `Update()` 시작 시 적용 (지연 전환). `Update(dt)`가 게임 루프 구동.
- **`Scene`** (추상) — `GameObject` 소유. 이터레이터 무효화 방지를 위한 지연 소멸(`m_pendingDestroy`). 순서: `Awake()` → `Start()` → `Update(dt)` / `LateUpdate(dt)`.
- **`GameObject`** — 컴포넌트 이중 저장:
  - `m_components` — `vector<unique_ptr<Component>>` (소유권, 순차 순회)
  - `m_componentMap` — `unordered_map<type_index, Component*>` (O(1) 조회)
  - `AddComponent<T>(args...)`는 즉시 `Awake()` 호출. `GetComponent<T>()` O(1). `GetTransform()`은 직접 포인터 반환 (조회 없음).
- **`Component`** (추상) — 라이프사이클: `Awake()` → `Start()` → `Update(dt)` → `LateUpdate(dt)` → `OnDestroy()`. `GetOwner()`로 소유 GameObject 접근.
- **`Transform`** (Component 상속) — 위치/회전(오일러 각도)/스케일. `GetWorldMatrix()`는 더티 플래그 기반 지연 계산.

#### 렌더링 컴포넌트

- **`RenderComponent`** (Component 상속) — 렌더 가능 컴포넌트의 베이스. `shared_ptr<Render::Material>`, `Render::Geometry*`, `Render::RenderPass::Layer` 보유. `Update(dt)`가 `SetWorld()` / `SetViewProj()` 호출 후 `RenderPipeline::Submit()` 실행.
- **`QuadComponent`** (GameEngineLib 버전, RenderComponent 상속) — `Awake()`에서 `GeometryManager`로부터 미리 로드된 `"Quad"` 지오메트리 조회. 없으면 어서트(`InitEngine()` 선행 필수).
- **`Quad`** (Render::Geometry 상속) — `DefaultCreateBuffers()`가 `VTXPOSTEX` 버텍스와 `uint16_t` 인덱스(6개, CCW 삼각형 2개)로 원점 중심의 단위 쿼드 생성.

#### 버텍스 포맷 (`EngineStruct.h`, namespace `GameEngine`)

| 구조체 | 요소 | D3D 시맨틱 |
|--------|------|------------|
| `VTXPOS` | `XMFLOAT3 position` | `POSITION` |
| `VTXPOSCOL` | `XMFLOAT3 position`, `XMFLOAT4 color` | `POSITION`, `COLOR` |
| `VTXPOSTEX` | `XMFLOAT3 position`, `XMFLOAT2 texcoord` | `POSITION`, `TEXCOORD` |

각 구조체는 정적 `sElements[]`(`D3D11_INPUT_ELEMENT_DESC`)를 노출하며, `ShaderGroup::Initialize()`가 이를 사용해 `InputLayout` 생성.

#### 리소스 매니저

세 매니저 모두 `ResourceCache<T>`(`JobQueue` 상속)를 상속하는 싱글톤. `Lock`으로 스레드 안전.

- **`ResourceCache<T>`** (템플릿 베이스) — `ResourceState` 열거형: `NotLoaded → Loading → Ready / Failed`. `ReadLockGuard` / `WriteLockGuard` 사용. `GetCached(key)` (Ready만 반환), `GetReserved(key)` (Loading/Ready 반환).
- **`GeometryManager`** — `LoadSync(key, geo)`가 메인 스레드에서 VB/IB 생성 후 캐시. `LoadAsync(key, geo, cb)`는 I/O를 워커 스레드에 오프로드 후 GPU 생성은 메인 스레드로 디스패치. `Get(key)`는 캐시된 지오메트리의 로우 포인터 반환 (공유·불변).
- **`ShaderManager`** — `LoadSync(key, vsPath, psPath)`가 VS/PS 로드 후 `ShaderGroup` 생성. 내부 VS/PS 캐시(`m_vs`, `m_ps`)로 셰이더 재사용. `Get(key)`는 캐시된 ShaderGroup의 로우 포인터 반환.
- **`MaterialManager`** — `LoadSync(key, mat, shaderKey)`가 ShaderGroup 준비를 기다린 뒤 `Material::Initialize(sg)` 호출. **`Get(key)`는 `unique_ptr` 클론 반환** — 호출자는 GPU 버퍼를 공유하되 독립적인 인스턴스를 받음.

#### 타이머

`GameEngine::Timer` (`Timer.h`) — QPC 기반 프레임 타이머. `SetTargetFPS(n)` → `Tick()` → `IsFrameReady()` (어큐뮬레이터 + 스핀 대기). `GetDeltaTime()`은 마지막 프레임 레디 이후 경과 시간(초). `GetFPS()`는 약 1초 단위 갱신.

---

### Client

게임 진입점과 구체적인 Scene·Component 구현체.

- **`Client.cpp`** — `WinMain()`. 윈도우 생성, `MainApp` 인스턴스화, 메시지 루프 실행.
- **`ClientGlobal`** — `InitGame()` / `ReleaseGame()`. Core → Render → GameEngine 순 초기화.
- **`MainApp`** — `Init()`에서 `timeBeginPeriod(1)`, `InitEngine()`, `InitRender()`, `LoadScene<QuadColorScene>()`, 타이머 설정 순 실행. `Loop()`가 엄격한 프레임 순서를 강제. `m_camID`(ViewProjManager 카메라 ID)와 `m_timer` 보유.
- **`QuadColorScene`** — 샘플 씬: `Awake()`에서 `DefaultColorMaterial`(빨간색, 파란색) 두 개를 생성하고 각각 `QuadComponent`를 부착. 파란 쿼드는 `SetPosition(0, 1, 0)`으로 Y 오프셋 적용.
- **`QuadComponent`** (Client 버전, RenderComponent 상속) — GameEngineLib 버전과 별도. 생성자에서 `shared_ptr<Render::Material>`을 직접 받음.

---

## 새로운 렌더 가능 컴포넌트 추가 방법

1. `RenderLib`에 `Material` 서브클래스 작성. `Initialize(ShaderGroup*)`에서 `AutomaticRegisterVS` / `AutomaticRegisterPS`로 CB 등록. `SetWorld()` / `SetViewProj()` 및 커스텀 세터 오버라이드.
2. 새 셰이더 쌍이 필요하면 `ShaderManager::LoadSync(key, vsPath, psPath)`로 로드.
3. `RenderComponent` 상속 컴포넌트 작성. `Awake()`에서 지오메트리(`GeometryManager::Get()`)와 머티리얼(`MaterialManager::Get()`) 설정. 제출은 `RenderComponent::Update(dt)`가 자동 처리.
4. `Scene::Awake()`에서 GameObject에 컴포넌트 부착.

---

## 예정 작업

`Plan.md` (260305) 기준:

- **Material Release 정리** — `Material`의 현재 `Release()` 경로를 제거. `ConstantBufferManager`가 버퍼를 레퍼런스 카운트 방식 대신 `unique_ptr`로 보유하도록 변경.
- **Clone 수정** — `Clone()` 시 `ConstantBufferParameter::m_pData` 포인터가 원본 멤버가 아닌 클론의 멤버를 가리키도록 교체. `Clone()`에서 `Initialize()` 호출 제거.
- **`ViewProjManager` 제거** — 삭제 예정. 뷰/프로젝션 데이터는 다른 방식으로 관리.
- **`DefaultColorMaterial`을 Client / Tool로 이동** — `RenderLib`에 있어선 안 됨. 소비자 계층으로 이동.
- **Assimp 모델 로딩** — Assimp로 메시 데이터를 로드하고 커스텀 파싱 파이프라인을 거쳐 `Geometry` 서브클래스로 변환.
