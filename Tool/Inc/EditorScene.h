#pragma once

#ifdef TOOL

#include "Scene.h"
#include "imgui.h"
#include "ImGuizmo.h"

namespace GameEngine
{
	class EditorScene : public Scene
	{
	public:
		void Update(float dt) override;

	protected:
		// 서브클래스가 Inspector 하단에 추가 UI를 그리는 확장점
		virtual void OnInspectorFooter(GameObject* selected) {}

	private:
		void BuildHierarchyPanel();
		void BuildInspectorPanel();

		GameObject*         m_selectedGO = nullptr;
		ImGuizmo::OPERATION m_gizmoOp   = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE      m_gizmoMode = ImGuizmo::WORLD;
	};
}

#endif // TOOL
