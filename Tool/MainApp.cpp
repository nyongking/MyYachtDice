#include "ClientPch.h"
#include "MainApp.h"

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

#include "ThreadManager.h"
#include "MainThreadQueue.h"

#include "Renderer.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "SceneManager.h"
#include "CameraComponent.h"

#include "TestScene.h"
#include "ModelLoadScene.h"

void MainApp::ImGuiInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(wnd);
	ImGui_ImplDX11_Init(
		Render::RenderDevice::GetInstance().GetDevice().Get(),
		Render::RenderDevice::GetInstance().GetContext().Get());
}

void MainApp::ImGuiShutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void MainApp::ImGuiBeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void MainApp::ImGuiEndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}


MainApp::MainApp()
{
}

MainApp::~MainApp()
{
	ImGuiShutdown();
	timeEndPeriod(1);
	ReleaseGame();
	ReleaseCore();
}

void DoWorkerJob()
{
	const uint64 WORKER_TICK = 25;
	const uint64 WAIT_TICK = 3;
	while (GRunning)
	{
		DoGlobalWork(WORKER_TICK, WAIT_TICK);
	}
}

bool MainApp::Init()
{
	LThreadID = 0;
	for (uint32 i = 0; i < 5; ++i)
	{
		GThreadManager->Launch([this]() {DoWorkerJob(); });
	}

	InitGame();

	GameEngine::SceneManager::GetInstance().LoadScene<TestScene>();

	ImGuiInit();

	timeBeginPeriod(1);
	m_timer.Reset();
	m_timer.SetTargetFPS(144);

	return true;
}

void MainApp::Loop()
{
	m_timer.Tick();
	if (!m_timer.IsFrameReady())
	{
		Sleep(1);
		return;
	}

	const float dt = m_timer.GetDeltaTime();

	ImGuiBeginFrame();                                            // ① ImGui 새 프레임

	Render::RenderPipeline::GetInstance().BeginFrame();           // ② 게임 큐 초기화

	// ── Game Update + Submit ──
	GameEngine::SceneManager::GetInstance().Update(dt);

	// ── Render ──
	Render::Renderer::GetInstance().RenderBegin();
	Render::RenderPipeline::GetInstance().Execute(
		Render::RenderDevice::GetInstance().GetContext().Get());
	ImGuiEndFrame();                                              // ④ ImGui 드로우 (게임 위에)
	Render::Renderer::GetInstance().RenderEnd();
	Render::RenderPipeline::GetInstance().EndFrame();

	GMainQueue->ExecuteAll();

#ifdef _DEBUG
	wsprintf(m_szBuf, TEXT("FPS: %d / dt: %.4f"), m_timer.GetFPS(), dt);
	SetWindowText(wnd, m_szBuf);
#endif
}
