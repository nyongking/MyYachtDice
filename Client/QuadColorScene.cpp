#include "ClientPch.h"
#include "QuadColorScene.h"
#include "QuadComponent.h"
#include "RenderDefaultRegistry.h"
#include "DefaultColorMaterial.h"
#include "CameraComponent.h"
#include "SceneManager.h"


void TestScene::Awake()
{
	// 카메라 생성
	{
		using namespace DirectX;
		auto* camGO = CreateGameObject("MainCamera");
		auto* cam   = camGO->AddComponent<GameEngine::CameraComponent>();

		cam->SetLookAt(XMVectorSet(0.f, 0.f, -5.f, 1.f),
		               XMVectorSet(0.f, 0.f,  0.f, 1.f),
		               XMVectorSet(0.f, 1.f,  0.f, 0.f));
		cam->SetPerspective(XM_PIDIV4, 16.f / 9.f, 0.1f, 1000.f);

		GameEngine::SceneManager::GetInstance().SetMainCamera(cam);
	}

	auto& reg = Render::RenderDefaultRegistry::GetInstance();

	{
		auto redMat = std::make_shared<Render::DefaultColorMaterial>();
		redMat->Initialize(reg.GetShaderGroup(Render::SHADER_QUAD_TEX).get());
		redMat->SetColor({ 1.f, 0.f, 0.f, 1.f }); // 빨간색

		auto* go = CreateGameObject("Quad");
		go->AddComponent<QuadComponent>(redMat);
	}

	{
		auto blueMat = std::make_shared<Render::DefaultColorMaterial>();
		blueMat->Initialize(reg.GetShaderGroup(Render::SHADER_QUAD_TEX).get());
		blueMat->SetColor({ 0.f, 0.5f, 1.f, 1.f }); // 파란색

		auto* go = CreateGameObject("Quad");
		go->AddComponent<QuadComponent>(blueMat);

		go->GetTransform()->SetPosition(GameEngine::float3(0.f, 1.f, 0.f));
	}
}
