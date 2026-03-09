#include "ClientPch.h"
#include "ClientGlobal.h"
#include "EngineGlobal.h"
#include "ShaderManager.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "DeferredLightingMaterial.h"

bool GRunning = true;

// DeferredLightingMaterial 수명 관리 (RenderPipeline은 raw ptr만 보관)
static std::unique_ptr<Render::DeferredLightingMaterial> s_lightingMat;

void InitGame()
{
    GameEngine::InitEngine();

    // ── 셰이더 로드 ──
    GameEngine::GShaderManager->LoadSync(
        "GBuffer",
        "./Bin/Resource/Shader/VSGBUFFER.hlsl",
        "./Bin/Resource/Shader/PSGBUFFER.hlsl");

    auto* lightingSG = GameEngine::GShaderManager->LoadSync(
        "DeferredLighting",
        "./Bin/Resource/Shader/VSFULLSCREEN.hlsl",
        "./Bin/Resource/Shader/PSLIGHTING.hlsl");

    // ── DeferredLightingMaterial 생성 + 파이프라인에 연결 ──
    if (lightingSG)
    {
        auto* device = Render::RenderDevice::GetInstance().GetDevice().Get();

        s_lightingMat = std::make_unique<Render::DeferredLightingMaterial>();
        if (s_lightingMat->Initialize(lightingSG, device))
            Render::RenderPipeline::GetInstance().SetLightingMaterial(s_lightingMat.get());
    }
}

void ReleaseGame()
{
    s_lightingMat.reset();
    GameEngine::ReleaseEngine();
}
