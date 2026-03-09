#include "ClientPch.h"
#include "QuadColorScene.h"

#include "CameraComponent.h"
#include "SceneManager.h"

void TestScene::Awake()
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
