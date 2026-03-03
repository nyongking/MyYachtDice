#pragma once
#include "RenderItem.h"

namespace Render
{
	class RenderPass
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

		explicit RenderPass(Layer layer) : m_layer(layer) {}
		~RenderPass() = default;

	public:
		void SetRenderTarget(class RenderTargetGroup* rtGroup);

		// 게임 오브젝트가 제출
		void Submit(const RenderCommand& cmd);

		// Renderer가 프레임마다 호출
		void Execute(ID3D11DeviceContext* ctx);
		void Clear();

	private:
		void Sort();

		Layer                      m_layer;
		class RenderTargetGroup*   m_renderTarget = nullptr;
		std::vector<RenderCommand> m_queue;
	};
}
