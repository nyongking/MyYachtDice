#include "GameEnginePch.h"
#include "GameObject.h"

namespace GameEngine
{
	GameObject::GameObject()
	{
		m_transform          = std::make_unique<Transform>();
		m_transform->m_owner = this;
	}

	void GameObject::Update(float dt)
	{
		if (!m_active)
			return;

		for (auto& comp : m_components)
			comp->Update(dt);
	}

	void GameObject::LateUpdate(float dt)
	{
		if (!m_active)
			return;

		for (auto& comp : m_components)
			comp->LateUpdate(dt);
	}
}
