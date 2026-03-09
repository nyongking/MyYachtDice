#pragma once

namespace Render
{
	class RenderTargetGroup;

	class RenderPassBase
	{
	public:
		enum class Layer
		{
			Opaque      = 0,
			Transparent = 1,
			UI          = 2,
			Effect      = 3,
			COUNT
		};

		virtual ~RenderPassBase() = default;

		virtual void Execute(ID3D11DeviceContext* ctx) = 0;
		virtual void Clear() {}

		void SetRenderTarget(RenderTargetGroup* rtg) { m_renderTarget = rtg; }

	protected:
		RenderTargetGroup* m_renderTarget = nullptr;
	};
}
