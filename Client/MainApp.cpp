#include "ClientPch.h"
#include "MainApp.h"

#include "ThreadManager.h"
#include "MainThreadQueue.h"

#include "Renderer.h"
#include "RenderResourceManager.h"

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
	//CoInitialize(nullptr);

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
	

	

	return true;
}

void MainApp::Loop()
{
	float time = 0.f;

	Render::Renderer::GetInstance().RenderBegin();
	Render::Renderer::GetInstance().RenderEnd();

	/*if (m_frameTimer.IsUpdate(&time))
	{
		UpdateInput();

		if (KeyState(LBUTTON, KEY_DOWN))
		{
			ChangeScene(1, 0, nullptr);

		}

		UpdateScene(time);
		RenderScene();

		RenderBegin();
		Render();
		RenderEnd();

		EndofScene();

#ifdef _DEBUG
		{
			USHORT fps = 0, maxfps = 0;
			
			m_frameTimer.GetFps(&fps, &maxfps);
			swprintf_s(m_szBuf, L"FPS : %d, MaxFPS : %d", fps, maxfps);
			SetWindowText(GetWnd(), m_szBuf);
		}
#endif
	}*/

	GMainQueue->ExecuteAll();

}
