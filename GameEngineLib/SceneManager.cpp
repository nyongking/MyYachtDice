#include "GameEnginePch.h"
#include "SceneManager.h"

namespace GameEngine
{
	void SceneManager::Update(float dt)
	{
		ApplyPendingScene();

		if (m_currentScene)
			m_currentScene->Update(dt);
	}

	void SceneManager::ApplyPendingScene()
	{
		if (!m_pendingScene)
			return;

		m_currentScene = std::move(m_pendingScene);
		m_currentScene->Awake();
		m_currentScene->Start();
	}
}
