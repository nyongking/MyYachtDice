#pragma once
#include "Timer.h"
#include "imgui.h"
#include "ImGuizmo.h"

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

	GameEngine::Timer m_timer;

#ifdef _DEBUG
	TCHAR m_szBuf[64];
#endif
};

