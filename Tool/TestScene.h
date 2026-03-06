#pragma once
#include "Scene.h"
#include "GameObject.h"
#include "imgui.h"
#include "ImGuizmo.h"

class TestScene : public GameEngine::Scene
{
public:
	void Awake()           override;
	void Update(float dt)  override;

private:
	GameEngine::GameObject* m_editTarget = nullptr;
	ImGuizmo::OPERATION     m_gizmoOp   = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE          m_gizmoMode = ImGuizmo::WORLD;
};
