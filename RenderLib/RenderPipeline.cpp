#include "RenderPch.h"
#include "RenderPipeline.h"

#include "RenderDevice.h"
#include "Renderer.h"

namespace Render
{
	bool RenderPipeline::Initialize(UINT sizeX, UINT sizeY)
	{
		auto device = RenderDevice::GetInstance().GetDevice();

		// Geometry 패스 생성 및 초기화
		for (int i = 0; i < PASS_COUNT; ++i)
		{
			m_passes[i] = std::make_unique<GeometryPass>(static_cast<RenderPassBase::Layer>(i));
			if (!m_passes[i]->Initialize(device.Get()))
				return false;
		}

		// G-Buffer MRT 생성 (Albedo+Metallic / Normal+Roughness / WorldPos)

		m_gBuffer = std::make_unique<RenderTargetGroup>();
		if (!m_gBuffer->Create(device.Get(), sizeX, sizeY,
			{
				{ DXGI_FORMAT_R16G16B16A16_FLOAT },  // RT0: Albedo + Metallic
				{ DXGI_FORMAT_R16G16B16A16_FLOAT },  // RT1: Normal + Roughness
				{ DXGI_FORMAT_R32G32B32A32_FLOAT },  // RT2: World Position
			}, true))
			return false;

		// Opaque 패스 → G-Buffer 렌더 타겟 연결
		m_passes[static_cast<int>(RenderPassBase::Layer::Opaque)]->SetRenderTarget(m_gBuffer.get());

		// Lighting 패스 생성 및 G-Buffer 연결
		m_lightingPass = std::make_unique<LightingPass>();
		m_lightingPass->SetGBuffer(m_gBuffer.get());

		return true;
	}

	void RenderPipeline::Submit(RenderPassBase::Layer layer, const RenderCommand& cmd)
	{
		int idx = static_cast<int>(layer);
		if (idx < 0 || idx >= PASS_COUNT)
			return;

		m_passes[idx]->Submit(cmd);
	}

	void RenderPipeline::SubmitLight(const LightCommand& cmd)
	{
		m_lightingPass->SubmitLight(cmd);
	}

	void RenderPipeline::BeginFrame()
	{
		// G-Buffer 클리어
		if (m_gBuffer)
		{
			auto ctx = RenderDevice::GetInstance().GetContext();
			m_gBuffer->Clear(ctx.Get(), float4(0.f, 0.f, 0.f, 0.f));
		}
	}

	void RenderPipeline::Execute(ID3D11DeviceContext* ctx)
	{
		// 1. GBuffer Pass — Opaque 오브젝트를 G-Buffer MRT에 기록
		m_passes[static_cast<int>(RenderPassBase::Layer::Opaque)]->Execute(ctx);

		// 2. Lighting Pass — G-Buffer SRV 읽기, 백버퍼에 조명 출력
		Renderer::GetInstance().BindBackbuffer(ctx);
		m_lightingPass->Execute(ctx);

		// 3. Transparent — Forward, 백버퍼 직접 출력
		m_passes[static_cast<int>(RenderPassBase::Layer::Transparent)]->Execute(ctx);

		// 4. UI
		m_passes[static_cast<int>(RenderPassBase::Layer::UI)]->Execute(ctx);

		// 5. Effect
		m_passes[static_cast<int>(RenderPassBase::Layer::Effect)]->Execute(ctx);
	}

	void RenderPipeline::EndFrame()
	{
		for (auto& pass : m_passes)
			pass->Clear();

		m_lightingPass->Clear();
	}
}
