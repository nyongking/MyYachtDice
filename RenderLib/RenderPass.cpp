#include "RenderPch.h"
#include "RenderPass.h"

#include "RenderTargetGroup.h"
#include "ShaderGroup.h"
#include "Material.h"
#include "Geometry.h"

#include <algorithm>

namespace Render
{
	void RenderPass::SetRenderTarget(RenderTargetGroup* rtGroup)
	{
		m_renderTarget = rtGroup;
	}

	void RenderPass::Submit(const RenderCommand& cmd)
	{
		m_queue.push_back(cmd);
	}

	void RenderPass::Execute(ID3D11DeviceContext* ctx)
	{
		if (m_queue.empty())
			return;

		// Stage 3: RenderTarget 바인드 (nullptr이면 backbuffer 그대로 사용)
		if (m_renderTarget)
			m_renderTarget->Bind(ctx);

		// 정렬 (Stage 2+4 상태 변경 최소화)
		Sort();

		ShaderGroup* currentShader = nullptr;

		for (const auto& cmd : m_queue)
		{
			if (nullptr == cmd.geometry || nullptr == cmd.material)
				continue;

			ShaderGroup* sg = cmd.material->GetShaderGroup();

			// Stage 2 + Stage 4: InputLayout + Shader 변경 시에만 바인드
			if (sg != currentShader)
			{
				if (sg)
					sg->BindShaderAndLayout(ctx);
				currentShader = sg;
			}

			// Stage 5: Material CBs 바인드
			cmd.material->BindMaterial();

			// Stage 6: Geometry VB/IB 바인드 + DrawIndexed
			cmd.geometry->BindAndDraw(ctx);
		}
	}

	void RenderPass::Clear()
	{
		m_queue.clear();
	}

	void RenderPass::Sort()
	{
		// ShaderGroup 포인터 기준 정렬 → 동일 셰이더 드로우 묶기 (상태 최소화)
		std::stable_sort(m_queue.begin(), m_queue.end(),
			[](const RenderCommand& a, const RenderCommand& b)
			{
				ShaderGroup* sgA = (a.material) ? a.material->GetShaderGroup() : nullptr;
				ShaderGroup* sgB = (b.material) ? b.material->GetShaderGroup() : nullptr;
				return sgA < sgB;
			});
	}
}
