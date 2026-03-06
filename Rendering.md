## 렌더링 정렬 우선순위 (무거운 순)

### 1단계: 층(Layer) 및 패스(Pass) 구분

- **내용:** UI, 불투명(Opaque), 반투명(Transparent), 이펙트 등.
- **이유:** 불투명은 앞에서 뒤로(Front-to-Back) 그려서 픽셀 연산을 아끼고, 반투명은 뒤에서 앞으로(Back-to-Front) 그려야 블렌딩이 깨지지 않습니다. 이건 성능을 넘어 **'결과물의 정확도'** 문제라 최우선입니다.

### 2단계: 파이프라인 설정 (Input Layout & Vertex Type)

- **내용:** Vertex 타입.
- **이유:** `IASetInputLayout`은 파이프라인을 재구성하므로 매우 무겁습니다. 같은 정점 구조를 쓰는 메쉬들을 모아야 합니다.

### 3단계: 렌더 타겟 설정 (MRT)

- **내용:** G-Buffer(지연 렌더링)인지, 일반 포워드 렌더링인지 등.
- **이유:** `OMSetRenderTargets`는 GPU 캐시를 플러시할 수 있어 자주 바꾸면 안 됩니다.

### 4단계: 셰이더 프로그램 (Shader State)

- **내용:** `VSSetShader`, `PSSetShader`.
- **이유:** 셰이더 코드가 바뀌면 GPU 내부의 실행 유닛들이 재배치됩니다.

### 5단계: 리소스 바인딩 (SRV - Texture)

- **내용:** Texture(SRV) 단계.
- **이유:** 같은 셰이더를 쓰더라도 텍스처가 다르면 여기서 갈립니다. 텍스처별로 정렬하면 `PSSetShaderResources` 호출을 최소화할 수 있습니다.

### 6단계: 개별 데이터 업데이트 (CBuffer Update & Draw)

- **내용:** World 행렬, 개별 Color Material 등.
- **이유:** 가장 안쪽 루프입니다. 여기서 `Map/Discard`로 데이터를 밀어 넣고 `Draw`를 호출합니다.

---

## Deferred Shading (MRT 기법)

### 개요

모델 렌더링에 Deferred Shading을 도입하기 위해 MRT(Multiple Render Targets) 기법을 사용한다.
핵심은 렌더링을 두 단계로 분리하는 것이다:

1. **Geometry Pass (G-Buffer)** — 오브젝트의 Albedo, Normal, Position 등을 각각 별도 렌더 타겟에 기록
2. **Lighting Pass** — G-Buffer SRV를 읽어 픽셀별 조명 계산, 결과를 백버퍼에 출력

### 프레임 순서

```
[현재]
BeginFrame → SceneManager::Update → RenderBegin → Execute(Opaque/Transparent/UI/Effect) → RenderEnd → EndFrame

[변경 후]
BeginFrame
  SceneManager::Update → Submit(GBuffer, cmd), SubmitLight(cmd), Submit(Transparent, cmd)
RenderBegin (backbuffer 클리어)
  [1단계] GBuffer Pass   → MRT(Albedo/Normal/WorldPos) + Depth
  [2단계] Lighting Pass  → G-Buffer SRV 읽기 → backbuffer에 조명 출력
  [3단계] Transparent    → Forward 방식, backbuffer 직접 출력 (Depth 공유)
  [4단계] UI
RenderEnd → Present
EndFrame
```

### G-Buffer 구성 (MRT 3개)

```
RT0: RGBA16F  → Albedo(RGB) + Metallic(A)
RT1: RGBA16F  → WorldNormal(RGB) + Roughness(A)
RT2: RGBA32F  → WorldPosition(RGB) + (A 미사용 또는 AO)
DSV: D24S8    → Depth (Transparent Pass와 공유)
```

---

## RenderPass 상속 구조

패스마다 Execute 로직과 Submit 방식이 본질적으로 다르기 때문에, 단일 RenderPass 클래스에 전부 넣는 대신 상속 구조로 분리한다.

### 패스별 차이

| 패스 | Execute 내용 | Submit 방식 |
|------|-------------|------------|
| GeometryPass | sort → bind shader → bind material → DrawIndexed | RenderCommand 큐 |
| LightingPass | fullscreen quad + 라이트 CB 배열 + G-Buffer SRV | LightCommand 큐 |
| PostProcessPass | fullscreen quad + 이전 패스 SRV | 별도 없음 |

### 클래스 구조

```cpp
// Base: 렌더 타겟 관리 + Execute 인터페이스만 정의
class RenderPassBase
{
public:
    virtual void Execute(ID3D11DeviceContext* ctx) = 0;
    virtual void Clear() {}
    void SetRenderTarget(RenderTargetGroup* rtg);

protected:
    RenderTargetGroup* m_renderTarget = nullptr;  // nullptr = backbuffer
};

// 기존 RenderPass 역할: Geometry 큐 방식 유지
class GeometryPass : public RenderPassBase
{
    void Submit(const RenderCommand& cmd);
    void Execute(ID3D11DeviceContext* ctx) override;  // sort + draw
    void Clear() override;
    std::vector<RenderCommand> m_queue;
};

// Deferred Lighting 전용 패스
class LightingPass : public RenderPassBase
{
    void SubmitLight(const LightCommand& cmd);
    void SetGBuffer(RenderTargetGroup* gbuffer);
    void Execute(ID3D11DeviceContext* ctx) override;  // fullscreen quad + 라이트 CB
    void Clear() override;
    RenderTargetGroup*        m_gBuffer = nullptr;
    std::vector<LightCommand> m_lights;
};

// 범용 후처리 패스 (블룸, 톤매핑 등)
class PostProcessPass : public RenderPassBase
{
    void SetInputSRV(uint32_t slot, ID3D11ShaderResourceView* srv);
    void Execute(ID3D11DeviceContext* ctx) override;
};
```

### RenderPipeline 구조

패스 간 리소스 흐름(SRV 주입 등)을 명시적으로 연결해야 하므로, 구체 타입 포인터를 보유한다.
패스 수가 적으니 다형성 컬렉션보다 명시적 멤버가 더 명확하다.

```cpp
class RenderPipeline
{
public:
    void Submit(Layer layer, const RenderCommand& cmd);
    void SubmitLight(const LightCommand& cmd);

    void Execute(ID3D11DeviceContext* ctx)
    {
        m_gbufferPass->Execute(ctx);

        m_lightingPass->SetGBuffer(m_gBuffer.get());
        m_lightingPass->Execute(ctx);

        m_transparentPass->Execute(ctx);
        m_uiPass->Execute(ctx);
    }

private:
    std::unique_ptr<RenderTargetGroup> m_gBuffer;

    std::unique_ptr<GeometryPass>    m_gbufferPass;
    std::unique_ptr<LightingPass>    m_lightingPass;
    std::unique_ptr<GeometryPass>    m_transparentPass;
    std::unique_ptr<GeometryPass>    m_uiPass;
};
```

---

## 구현 시 주의사항

- **Transparent와 Depth 공유**: G-Buffer Pass의 DSV를 Transparent Pass에서도 사용. `RenderTargetGroup::GetDSV()` 추가 필요.
- **G-Buffer SRV 언바인딩**: Lighting Pass 실행 후, 다음 프레임 G-Buffer Pass 전에 SRV 슬롯을 nullptr로 해제해야 D3D11 경고 방지.
- **G-Buffer 클리어**: `BeginFrame()`에서 G-Buffer RenderTargetGroup도 클리어.
- **G-Buffer 소유**: `RenderPipeline::Initialize(sizeX, sizeY)` 시그니처 확장 → 내부에서 `RenderTargetGroup` 생성.

---

## 필요한 추가 리소스

| 항목 | 내용 |
|------|------|
| `VSGBUFFER.hlsl` | World/ViewProj 변환, Normal/Tangent 변환 |
| `PSGBUFFER.hlsl` | SV_Target0/1/2에 Albedo, Normal, WorldPos 출력 |
| `VSFULLSCREEN.hlsl` | NDC 풀스크린 쿼드 패스스루 |
| `PSLIGHTING.hlsl` | G-Buffer SRV 읽기 + LightData 배열 CB로 조명 계산 |
| `GBufferMaterial` | VTXPOSNORMTANUV 기반, Albedo/Normal Map 슬롯 포함 |
| `DeferredLightingMaterial` | G-Buffer SRV 3개 + LightData 배열 CB |
| Fullscreen Quad Geometry | NDC -1~+1 쿼드, VTXPOSTEX 포맷 |
