#include "GameEnginePch.h"
#include "SceneManager.h"
#include "PhysicsManager.h"
#include "EngineGlobal.h"

namespace GameEngine
{
	void SceneManager::Update(float dt)
	{
		ApplyPendingScene();

		if (GPhysicsManager)
			GPhysicsManager->Step(dt);

		if (m_currentScene)
		{
			m_currentScene->Update(dt);
			m_currentScene->LateUpdate(dt);
		}
	}

	void SceneManager::ApplyPendingScene()
	{
		if (!m_pendingScene)
			return;

		// 기존 씬의 물리 오브젝트 정리
		if (GPhysicsManager)
			GPhysicsManager->Clear();

		m_mainCamera   = nullptr;
		m_currentScene = std::move(m_pendingScene);
		m_currentScene->Awake();
		m_currentScene->Start();
	}
}
