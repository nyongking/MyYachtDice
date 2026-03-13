#pragma once

#include "JsonUtil.h"

#ifdef TOOL
#include "imgui.h"
#endif

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

		// 직렬화 인터페이스
		// Serialize()는 반드시 "type" 키를 포함해야 한다.
		virtual std::string GetTypeName()          const { return "Component"; }
		virtual MyJson      Serialize()            const { return { {"type", GetTypeName()} }; }
		virtual void        Deserialize(const MyJson&)   {}

		GameObject* GetOwner() const { return m_owner; }

		// Inspector GUI — TOOL 빌드에서만 실제 구현, 그 외에는 no-op.
		// vtable 레이아웃 일관성을 위해 항상 선언한다.
		virtual void OnInspectorGUI() {}


	private:
		friend class GameObject;
		GameObject* m_owner = nullptr;
	};
}
