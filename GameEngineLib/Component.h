#pragma once

namespace GameEngine
{
	class GameObject;

	class Component
	{
	public:
		virtual ~Component() = default;

		virtual void Awake()              {}
		virtual void Start()              {}
		virtual void Update(float dt)     {}
		virtual void LateUpdate(float dt) {}
		virtual void OnDestroy()          {}

		GameObject* GetOwner() const { return m_owner; }

	private:
		friend class GameObject;
		GameObject* m_owner = nullptr;
	};
}
