#include "ClientPch.h"
#include "MainApp.h"

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#include "ThreadManager.h"
#include "MainThreadQueue.h"

#include "Renderer.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "SceneManager.h"
#include "QuadColorScene.h"

MainApp::MainApp()
{
}

MainApp::~MainApp()
{
	timeEndPeriod(1);
	ReleaseGame();
	ReleaseServerCore();
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

	Render::RenderPipeline::GetInstance().BeginFrame();           // ① 큐 초기화 (Submit 전)

	// ── Game Update + Submit ──
	GameEngine::SceneManager::GetInstance().Update(dt);

	// ── Render ──
	Render::Renderer::GetInstance().RenderBegin();
	Render::RenderPipeline::GetInstance().Execute(
		Render::RenderDevice::GetInstance().GetContext());
	Render::Renderer::GetInstance().RenderEnd();
	Render::RenderPipeline::GetInstance().EndFrame();

	GMainQueue->ExecuteAll();

#ifdef _DEBUG
	wsprintf(m_szBuf, TEXT("FPS: %d / dt: %.4f"), m_timer.GetFPS(), dt);
	SetWindowText(wnd, m_szBuf);
#endif
}
