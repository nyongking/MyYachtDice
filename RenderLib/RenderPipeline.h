#pragma once
#include "GeometryPass.h"
#include "LightingPass.h"
#include "RenderTargetGroup.h"

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
		bool Initialize(UINT sizeX, UINT sizeY);

		// 초기화 후 1회 설정
		void SetLightingMaterial(class Material* mat) { m_lightingPass->SetMaterial(mat); }

		// 게임 오브젝트가 매 프레임 호출
		void Submit(RenderPassBase::Layer layer, const RenderCommand& cmd);
		void SubmitLight(const LightCommand& cmd);

		// Renderer가 프레임마다 호출
		void BeginFrame();
		void Execute(ID3D11DeviceContext* ctx);
		void EndFrame();

	private:
		RenderPipeline() = default;
		~RenderPipeline() = default;
		RenderPipeline(const RenderPipeline&)            = delete;
		RenderPipeline& operator=(const RenderPipeline&) = delete;

		// Geometry 계열 패스 (Opaque → G-Buffer, Transparent/UI/Effect → 백버퍼)
		static constexpr int PASS_COUNT = static_cast<int>(RenderPassBase::Layer::COUNT);
		std::array<std::unique_ptr<GeometryPass>, PASS_COUNT> m_passes;

		// Deferred 전용
		std::unique_ptr<RenderTargetGroup> m_gBuffer;
		std::unique_ptr<LightingPass>      m_lightingPass;
	};
}
