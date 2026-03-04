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
#include "ViewProjManager.h"
#include "SceneManager.h"
#include "QuadColorScene.h"

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

void MainApp::BuildToolUI()
{
	// ── 타깃 지연 초기화 ──────────────────────────────────────
	if (!m_editTarget)
	{
		auto* scene = dynamic_cast<QuadColorScene*>(
			GameEngine::SceneManager::GetInstance().GetCurrentScene());
		if (scene)
			m_editTarget = scene->GetEditTarget();
	}

	// ── Object Properties 창 ──────────────────────────────────
	ImGui::Begin("Object Properties");

	if (m_editTarget)
	{
		auto* tr = m_editTarget->GetTransform();

		// Operation 선택
		if (ImGui::RadioButton("Translate", m_gizmoOp == ImGuizmo::TRANSLATE))
			m_gizmoOp = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate",    m_gizmoOp == ImGuizmo::ROTATE))
			m_gizmoOp = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale",     m_gizmoOp == ImGuizmo::SCALE))
			m_gizmoOp = ImGuizmo::SCALE;

		// Mode 선택 (Scale은 항상 LOCAL)
		if (m_gizmoOp != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("World", m_gizmoMode == ImGuizmo::WORLD))
				m_gizmoMode = ImGuizmo::WORLD;
			ImGui::SameLine();
			if (ImGui::RadioButton("Local", m_gizmoMode == ImGuizmo::LOCAL))
				m_gizmoMode = ImGuizmo::LOCAL;
		}

		ImGui::Separator();

		// T/R/S 직접 편집
		float pos[3] = { tr->GetPosition().x, tr->GetPosition().y, tr->GetPosition().z };
		float rot[3] = { tr->GetRotation().x, tr->GetRotation().y, tr->GetRotation().z };
		float scl[3] = { tr->GetScale().x,    tr->GetScale().y,    tr->GetScale().z    };

		if (ImGui::DragFloat3("Position", pos, 0.01f))
			tr->SetPosition({ pos[0], pos[1], pos[2] });
		if (ImGui::DragFloat3("Rotation", rot, 0.5f))
			tr->SetRotation({ rot[0], rot[1], rot[2] });
		if (ImGui::DragFloat3("Scale",    scl, 0.01f))
			tr->SetScale({ scl[0], scl[1], scl[2] });
	}

	ImGui::End();

	// ── Gizmo ─────────────────────────────────────────────────
	if (!m_editTarget)
		return;

	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList(vp));
	ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);

	auto& vpm   = Render::ViewProjManager::GetInstance();
	const auto* pView = vpm.GetCurrentView();
	const auto* pProj = vpm.GetCurrentProj();
	if (!pView || !pProj)
		return;

	auto* tr = m_editTarget->GetTransform();
	float4x4 worldMat = tr->GetWorldMatrix();

	ImGuizmo::MODE mode = (m_gizmoOp == ImGuizmo::SCALE)
	                      ? ImGuizmo::LOCAL : m_gizmoMode;

	if (ImGuizmo::Manipulate(
	        reinterpret_cast<const float*>(pView),
	        reinterpret_cast<const float*>(pProj),
	        m_gizmoOp, mode,
	        reinterpret_cast<float*>(&worldMat)))
	{
		float t[3], r[3], s[3];
		ImGuizmo::DecomposeMatrixToComponents(
		    reinterpret_cast<float*>(&worldMat), t, r, s);

		tr->SetPosition({ t[0], t[1], t[2] });
		tr->SetRotation({ r[0], r[1], r[2] });
		tr->SetScale   ({ s[0], s[1], s[2] });
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

	BuildToolUI();                                                // ③ ImGui UI 빌드

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
