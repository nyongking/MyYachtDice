#include "GameEnginePch.h"
#include "Scene.h"

namespace GameEngine
{
	void Scene::Update(float dt)
	{
		for (auto& go : m_gameObjects)
			go->Update(dt);

		FlushPendingDestroy();
	}

	void Scene::LateUpdate(float dt)
	{
		for (auto& go : m_gameObjects)
			go->LateUpdate(dt);
	}

	GameObject* Scene::CreateGameObject(const std::string& name)
	{
		auto go = std::make_unique<GameObject>();
		go->SetName(name);
		GameObject* raw = go.get();
		m_gameObjects.push_back(std::move(go));
		return raw;
	}

	void Scene::DestroyGameObject(GameObject* go)
	{
		m_pendingDestroy.push_back(go);
	}

	void Scene::FlushPendingDestroy()
	{
		if (m_pendingDestroy.empty())
			return;

		for (GameObject* target : m_pendingDestroy)
		{
			auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
				[target](const std::unique_ptr<GameObject>& go)
				{
					return go.get() == target;
				});

			if (it != m_gameObjects.end())
				m_gameObjects.erase(it);
		}

		m_pendingDestroy.clear();
	}
}
