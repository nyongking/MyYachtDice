#include "ClientPch.h"
#include "ClientGlobal.h"

#include "EngineGlobal.h"
#include "ShaderManager.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "DeferredLightingMaterial.h"

#ifdef _DEBUG
#include "WireframeMaterial.h"
#endif

bool GRunning = true;

// DeferredLightingMaterial 수명 관리 (RenderPipeline은 raw ptr만 보관)
static std::unique_ptr<Render::DeferredLightingMaterial> s_lightingMat;

#ifdef _DEBUG
static std::unique_ptr<GameEngine::WireframeMaterial> s_wireframeMat;
#endif

void InitGame()
{
	GameEngine::InitEngine();

	// ── 셰이더 로드 (캐시되므로 Scene에서 Get()으로 재사용 가능) ──
	GameEngine::GShaderManager->LoadSync(
		"GBuffer",
		"Bin/Shader/VSGBUFFER.hlsl",
		"Bin/Shader/PSGBUFFER.hlsl");

	auto* lightingSG = GameEngine::GShaderManager->LoadSync(
		"DeferredLighting",
		"Bin/Shader/VSFULLSCREEN.hlsl",
		"Bin/Shader/PSLIGHTING.hlsl");

	// ── DeferredLightingMaterial 생성 + 파이프라인에 연결 ──
	if (lightingSG)
	{
		auto* device = Render::RenderDevice::GetInstance().GetDevice().Get();

		s_lightingMat = std::make_unique<Render::DeferredLightingMaterial>();
		if (s_lightingMat->Initialize(lightingSG, device))
			Render::RenderPipeline::GetInstance().SetLightingMaterial(s_lightingMat.get());
	}

#ifdef _DEBUG
	// 와이어프레임 셰이더 + 머티리얼 (Debug 빌드 전용)
	auto* wireSG = GameEngine::GShaderManager->LoadSync(
		"Wireframe",
		"Bin/Shader/VSWIREFRAME.hlsl",
		"Bin/Shader/PSWIREFRAME.hlsl");

	if (wireSG)
	{
		auto* device = Render::RenderDevice::GetInstance().GetDevice().Get();
		s_wireframeMat = std::make_unique<GameEngine::WireframeMaterial>();
		s_wireframeMat->Initialize(wireSG, device);
		GameEngine::GDebugWireframeMaterial = s_wireframeMat.get();
	}
#endif
}

void ReleaseGame()
{
#ifdef _DEBUG
	GameEngine::GDebugWireframeMaterial = nullptr;
	s_wireframeMat.reset();
#endif
	s_lightingMat.reset();
}
