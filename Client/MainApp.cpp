#include "ClientPch.h"
#include "MainApp.h"

#include "ThreadManager.h"
#include "MainThreadQueue.h"

#include "Renderer.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "ViewProjManager.h"
#include "SceneManager.h"
#include "QuadColorScene.h"

MainApp::MainApp()
{
}

MainApp::~MainApp()
{
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

	// 카메라 설정
	m_camID = Render::ViewProjManager::GetInstance().CreateViewProj();

	using namespace DirectX;
	XMMATRIX view = XMMatrixLookAtLH(
		XMVectorSet(0.f, 0.f, -5.f, 1.f),
		XMVectorSet(0.f, 0.f,  0.f, 1.f),
		XMVectorSet(0.f, 1.f,  0.f, 0.f));
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 16.f / 9.f, 0.1f, 1000.f);

	Render::ViewProjManager::GetInstance().UpdateViewProjByID(m_camID, view, proj);
	Render::ViewProjManager::GetInstance().ChangeCurrent(m_camID);

	GameEngine::SceneManager::GetInstance().LoadScene<QuadColorScene>();

	m_timer.Reset();
	m_timer.SetTargetFPS(60);

	return true;
}

void MainApp::Loop()
{
	m_timer.Tick();
	if (!m_timer.IsFrameReady())
		return;

	const float dt = m_timer.GetDeltaTime();

	Render::RenderPipeline::GetInstance().BeginFrame();           // ① 큐 초기화 (Submit 전)

	// ── Game Update + Submit ──
	GameEngine::SceneManager::GetInstance().Update(dt);

	// ── Render ──
	Render::Renderer::GetInstance().RenderBegin();
	Render::RenderPipeline::GetInstance().Execute(
		Render::RenderDevice::GetInstance().GetContext());
	Render::Renderer::GetInstance().RenderEnd();

	GMainQueue->ExecuteAll();

#ifdef _DEBUG
	wsprintf(m_szBuf, TEXT("FPS: %d  dt: %.4f"), m_timer.GetFPS(), dt);
	SetWindowText(wnd, m_szBuf);
#endif
}
