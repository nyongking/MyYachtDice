#pragma once
#include "RenderPassBase.h"
#include "LightItem.h"

namespace Render
{
	class Material;
	class RenderTargetGroup;

	class LightingPass : public RenderPassBase
	{
	public:
		LightingPass()  = default;
		~LightingPass() = default;

	public:
		void SubmitLight(const LightCommand& cmd);

		void SetGBuffer(RenderTargetGroup* gbuffer) { m_gBuffer = gbuffer; }
		void SetMaterial(Material* mat)             { m_material = mat; }

		void Execute(ID3D11DeviceContext* ctx) override;
		void Clear() override;

	private:
		RenderTargetGroup*        m_gBuffer  = nullptr;
		Material*                 m_material = nullptr;

		std::vector<LightCommand> m_lights;
	};
}
