#pragma once
#include "Timer.h"
#include "imgui.h"
#include "ImGuizmo.h"

namespace GameEngine { class GameObject; }

class MainApp
{
public:
	MainApp();
	~MainApp();

public:
	bool Init();
	void Loop();

private:
	void ImGuiInit();
	void ImGuiShutdown();
	void ImGuiBeginFrame();
	void ImGuiEndFrame();
	void BuildToolUI();

	uint32_t                m_camID     = 0;
	GameEngine::Timer       m_timer;

	GameEngine::GameObject* m_editTarget = nullptr;
	ImGuizmo::OPERATION     m_gizmoOp   = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE          m_gizmoMode = ImGuizmo::WORLD;

#ifdef _DEBUG
	TCHAR m_szBuf[64];
#endif
};

