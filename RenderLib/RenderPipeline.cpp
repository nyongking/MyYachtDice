#include "RenderPch.h"
#include "RenderPipeline.h"

namespace Render
{
	bool RenderPipeline::Initialize()
	{
		for (int i = 0; i < PASS_COUNT; ++i)
			m_passes[i] = std::make_unique<RenderPass>(static_cast<RenderPass::Layer>(i));

		return true;
	}

	void RenderPipeline::Submit(RenderPass::Layer layer, const RenderCommand& cmd)
	{
		int idx = static_cast<int>(layer);
		if (idx < 0 || idx >= PASS_COUNT)
			return;

		m_passes[idx]->Submit(cmd);
	}

	void RenderPipeline::BeginFrame()
	{
	}

	void RenderPipeline::Execute(ID3D11DeviceContext* ctx)
	{
		for (auto& pass : m_passes)
			pass->Execute(ctx);
	}

	void RenderPipeline::EndFrame()
	{
		for (auto& pass : m_passes)
			pass->Clear();
	}
}
