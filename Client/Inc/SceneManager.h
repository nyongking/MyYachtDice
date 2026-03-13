#pragma once

#include "Scene.h"

namespace GameEngine
{
	class CameraComponent;

	class SceneManager
	{
	public:
		static SceneManager& GetInstance()
		{
			static SceneManager instance;
			return instance;
		}

		template<typename T>
		void LoadScene()
		{
			static_assert(std::is_base_of<Scene, T>::value,
				"T must derive from Scene");
			m_pendingScene = std::make_unique<T>();
		}

		void   Update(float dt);
		Scene* GetCurrentScene() const { return m_currentScene.get(); }

		void             SetMainCamera(CameraComponent* cam) { m_mainCamera = cam; }
		CameraComponent* GetMainCamera() const               { return m_mainCamera; }

	private:
		SceneManager() = default;
		SceneManager(const SceneManager&)            = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		void ApplyPendingScene();

		std::unique_ptr<Scene> m_currentScene;
		std::unique_ptr<Scene> m_pendingScene;
		CameraComponent*       m_mainCamera = nullptr;
	};
}
