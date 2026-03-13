#pragma once

#include "GameObject.h"

namespace GameEngine
{
	class Scene
	{
	public:
		virtual ~Scene() = default;

		virtual void Awake() {}
		virtual void Start()  {}

		virtual void Update(float dt);
		virtual void LateUpdate(float dt);

		GameObject* CreateGameObject(const std::string& name = "GameObject");
		void        DestroyGameObject(GameObject* go);

		// JSON 씬 파일 저장 / 로드
		void SaveToFile(const std::string& path) const;
		void LoadFromFile(const std::string& path);

	protected:
		void FlushPendingDestroy();

		std::vector<std::unique_ptr<GameObject>> m_gameObjects;
		std::vector<GameObject*>                 m_pendingDestroy;
	};
}
