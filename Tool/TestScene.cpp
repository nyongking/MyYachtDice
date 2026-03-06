#include "ClientPch.h"
#include "TestScene.h"

#include "QuadTexMaterial.h"
#include "QuadComponent.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"


static constexpr const char* SHADER_KEY_POSTEX = "POSTEX";

void TestScene::Awake()
{
	auto& shaderMgr   = *GameEngine::ShaderManager::GetInstance();
	auto& materialMgr = *GameEngine::MaterialManager::GetInstance();
	auto& textureManager = *GameEngine::TextureManager::GetInstance();

	// Shader 로드
	shaderMgr.LoadSync(SHADER_KEY_POSTEX,
	                   "./Bin/Resource/Shader/VSPOSTEX.hlsl",
	                   "./Bin/Resource/Shader/PSPOSTEX.hlsl");

	// texture 로드
	auto tex = textureManager.LoadSync("TestTex", L"./Bin/Resource/Texture/Test01.dds");

	// Material 로드 — 캐시 원본의 Clone 반환
	auto matUnique = materialMgr.LoadSync(SHADER_KEY_POSTEX,
	                                      std::make_unique<QuadTexMaterial>(),
	                                      SHADER_KEY_POSTEX);
	if (!matUnique)
		return;

	// 색상 설정
	//matUnique->SetParam<float4>("Color", { 1.f, 0.f, 0.f, 1.f });

	// texture 설정
	matUnique->SetTexture("Texture", tex);

	// GameObject 생성 — QuadComponent::Awake()에서 "Quad" 지오메트리 자동 조회
	auto* go   = CreateGameObject("Quad");
	auto* quad = go->AddComponent<GameEngine::QuadComponent>();
	quad->SetMaterial(std::move(matUnique));

	m_editTarget = go;

	// 카메라 생성 — Quad 생성 이후에 SetFollowTarget 연결
	{
		auto* camGO = CreateGameObject("MainCamera");
		auto* cam   = camGO->AddComponent<GameEngine::CameraComponent>();

		cam->SetPerspective(DirectX::XM_PIDIV4, 16.f / 9.f, 0.1f, 1000.f);
		cam->SetFollowTarget(go->GetTransform(),
		                     { 0.f, 3.f, -8.f },   // eye offset
		                     { 0.f, 0.f,  0.f });  // lookAt offset (Quad 중심)
		cam->SetSmoothSpeed(5.f);                   // 부드러운 팔로우

		GameEngine::SceneManager::GetInstance().SetMainCamera(cam);
	}
}

void TestScene::Update(float dt)
{
	GameEngine::Scene::Update(dt);

	if (!m_editTarget)
		return;

	auto* tr = m_editTarget->GetTransform();

	// ── Object Properties 창 ──────────────────────────────────
	ImGui::Begin("Object Properties");

	if (ImGui::RadioButton("Translate", m_gizmoOp == ImGuizmo::TRANSLATE))
		m_gizmoOp = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", m_gizmoOp == ImGuizmo::ROTATE))
		m_gizmoOp = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", m_gizmoOp == ImGuizmo::SCALE))
		m_gizmoOp = ImGuizmo::SCALE;

	if (m_gizmoOp != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("World", m_gizmoMode == ImGuizmo::WORLD))
			m_gizmoMode = ImGuizmo::WORLD;
		ImGui::SameLine();
		if (ImGui::RadioButton("Local", m_gizmoMode == ImGuizmo::LOCAL))
			m_gizmoMode = ImGuizmo::LOCAL;
	}

	ImGui::Separator();

	float pos[3] = { tr->GetPosition().x, tr->GetPosition().y, tr->GetPosition().z };
	float rot[3] = { tr->GetRotation().x, tr->GetRotation().y, tr->GetRotation().z };
	float scl[3] = { tr->GetScale().x,    tr->GetScale().y,    tr->GetScale().z    };

	if (ImGui::DragFloat3("Position", pos, 0.01f))
		tr->SetPosition({ pos[0], pos[1], pos[2] });
	if (ImGui::DragFloat3("Rotation", rot, 0.5f))
		tr->SetRotation({ rot[0], rot[1], rot[2] });
	if (ImGui::DragFloat3("Scale",    scl, 0.01f))
		tr->SetScale({ scl[0], scl[1], scl[2] });

	ImGui::End();

	// ── Gizmo ─────────────────────────────────────────────────
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList(vp));
	ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);

	auto* cam = GameEngine::SceneManager::GetInstance().GetMainCamera();
	if (!cam)
		return;
	const auto* pView = cam->GetView();
	const auto* pProj = cam->GetProj();
	if (!pView || !pProj)
		return;

	float4x4 worldMat = tr->GetWorldMatrix();
	ImGuizmo::MODE mode = (m_gizmoOp == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : m_gizmoMode;

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
