#include "GameEnginePch.h"
#include "Scene.h"
#include <fstream>

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

	void Scene::SaveToFile(const std::string& path) const
	{
		MyJson j;
		MyJson goArray = MyJson::array();
		for (const auto& go : m_gameObjects)
			goArray.push_back(go->Serialize());
		j["gameObjects"] = goArray;

		std::ofstream file(path);
		file << j.dump(4);
	}

	void Scene::LoadFromFile(const std::string& path)
	{
		MyJson j;
		if (!LoadJson(path.c_str(), j)) return;
		if (!j.contains("gameObjects")) return;

		for (const auto& goJson : j["gameObjects"])
		{
			auto* go = CreateGameObject();
			go->Deserialize(goJson);
		}
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
