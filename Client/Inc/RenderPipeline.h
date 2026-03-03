#pragma once
#include "RenderPass.h"

namespace Render
{
	class RenderPipeline
	{
	public:
		static RenderPipeline& GetInstance()
		{
			static RenderPipeline instance;
			return instance;
		}

	public:
		bool Initialize();

		// 게임 오브젝트가 매 프레임 호출
		void Submit(RenderPass::Layer layer, const RenderCommand& cmd);

		// Renderer가 프레임마다 호출
		void BeginFrame();
		void Execute(ID3D11DeviceContext* ctx);
		void EndFrame();

	private:
		RenderPipeline() = default;
		~RenderPipeline() = default;
		RenderPipeline(const RenderPipeline&)            = delete;
		RenderPipeline& operator=(const RenderPipeline&) = delete;

		static constexpr int PASS_COUNT = static_cast<int>(RenderPass::Layer::COUNT);
		std::array<std::unique_ptr<RenderPass>, PASS_COUNT> m_passes;
	};
}
