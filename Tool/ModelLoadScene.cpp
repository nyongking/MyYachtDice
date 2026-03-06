#include "ClientPch.h"
#include "ModelLoadScene.h"

#include "ModelExporter.h"
#include "ShaderManager.h"
#include <fstream>
#include "MaterialManager.h"
#include "ModelManager.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "ModelComponent.h"
#include "DiceMaterial.h"
#include "GameObject.h"
#include "BufferStruct.h"

static constexpr const char* SHADER_KEY_MODEL = "POSNORMTANUV";

void ModelLoadScene::Awake()
{
	auto& shaderMgr = *GameEngine::ShaderManager::GetInstance();

	shaderMgr.LoadSync(SHADER_KEY_MODEL,
	                   "./Bin/Resource/Shader/VSPOSNORMTANUV.hlsl",
	                   "./Bin/Resource/Shader/PSDICE.hlsl");

	// 카메라
	auto* camGO = CreateGameObject("MainCamera");
	m_camera    = camGO->AddComponent<GameEngine::CameraComponent>();
	m_camera->SetPerspective(DirectX::XM_PIDIV4, 16.f / 9.f, 0.1f, 1000.f);
	ApplyCamera();
	GameEngine::SceneManager::GetInstance().SetMainCamera(m_camera);
}

void ModelLoadScene::Update(float dt)
{
	GameEngine::Scene::Update(dt);

	BuildCameraUI();
	BuildTransformUI();
	BuildMaterialUI();
	BuildModelLoaderUI();
	BuildModelInfoUI();
}

void ModelLoadScene::BuildCameraUI()
{
	ImGui::Begin("Camera");

	bool changed = false;
	changed |= ImGui::DragFloat3("Eye",    m_eye,    0.1f);
	changed |= ImGui::DragFloat3("Target", m_target, 0.1f);

	if (changed)
		ApplyCamera();

	ImGui::End();
}

void ModelLoadScene::ApplyCamera()
{
	if (!m_camera)
		return;

	m_camera->SetLookAt(
		DirectX::XMVectorSet(m_eye[0],    m_eye[1],    m_eye[2],    1.f),
		DirectX::XMVectorSet(m_target[0], m_target[1], m_target[2], 1.f),
		DirectX::XMVectorSet(0.f,         1.f,         0.f,         0.f));
}

void ModelLoadScene::BuildTransformUI()
{
	if (!m_modelGO)
		return;

	auto* tr = m_modelGO->GetTransform();

	// ── Transform 패널 ────────────────────────────────────────
	ImGui::Begin("Transform");

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
	if (!m_camera)
		return;
	const auto* pView = m_camera->GetView();
	const auto* pProj = m_camera->GetProj();
	if (!pView || !pProj)
		return;

	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList(vp));
	ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);

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

void ModelLoadScene::BuildMaterialUI()
{
	if (!m_model || m_model->GetSectionCount() == 0)
		return;

	ImGui::Begin("Material");

	for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
	{
		auto* mat = dynamic_cast<DiceMaterial*>(m_model->GetSection(i).material);
		if (!mat)
			continue;

		ImGui::PushID(static_cast<int>(i));

		std::string label = "Section " + std::to_string(i);
		ImGui::Text("%s", label.c_str());

		float color[4] = { mat->GetColor().x, mat->GetColor().y,
		                   mat->GetColor().z, mat->GetColor().w };

		if (ImGui::ColorEdit4("Color", color))
			mat->SetColor({ color[0], color[1], color[2], color[3] });

		ImGui::PopID();
		ImGui::Spacing();
	}

	ImGui::End();
}

void ModelLoadScene::BuildModelLoaderUI()
{
	ImGui::Begin("Model Loader");

	ImGui::InputText("Source", m_srcPath, sizeof(m_srcPath));
	ImGui::InputText("Output", m_dstPath, sizeof(m_dstPath));

	ImGui::Spacing();

	if (ImGui::Button("Export .mymesh"))
		DoExport();

	ImGui::SameLine();

	if (ImGui::Button("Load .mymesh"))
		DoLoad();

	if (!m_exportStatus.empty())
	{
		ImGui::Spacing();
		ImGui::Text("Export : %s", m_exportStatus.c_str());
	}

	if (!m_loadStatus.empty())
		ImGui::Text("Load   : %s", m_loadStatus.c_str());

	ImGui::End();
}

void ModelLoadScene::BuildModelInfoUI()
{
	if (!m_model || m_sectionInfos.empty())
		return;

	ImGui::Begin("Model Info");

	ImGui::Text("Sections : %zu", m_sectionInfos.size());
	ImGui::Separator();

	for (size_t i = 0; i < m_sectionInfos.size(); ++i)
	{
		const auto& sec = m_sectionInfos[i];
		if (ImGui::TreeNode(reinterpret_cast<void*>(i), "Section %zu", i))
		{
			ImGui::Text("matKey   : %s", sec.matKey.c_str());
			ImGui::Text("Vertices : %u", sec.vertexCount);
			ImGui::Text("Indices  : %u", sec.indexCount);
			ImGui::Separator();
			ImGui::Text("Min : (%.3f, %.3f, %.3f)", sec.minBound[0], sec.minBound[1], sec.minBound[2]);
			ImGui::Text("Max : (%.3f, %.3f, %.3f)", sec.maxBound[0], sec.maxBound[1], sec.maxBound[2]);
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void ModelLoadScene::ReadBoundsFromMymesh(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return;

	GameEngine::MeshFileHeader fileHeader = {};
	file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
	if (strncmp(fileHeader.magic, "MYMESH", 6) != 0)
		return;

	m_sectionInfos.clear();
	m_sectionInfos.resize(fileHeader.sectionCount);

	// 섹션 헤더만 읽어 bounds 추출 (정점/인덱스 데이터는 건너뜀)
	for (uint32_t i = 0; i < fileHeader.sectionCount; ++i)
	{
		GameEngine::MeshSectionHeader secHeader = {};
		file.read(reinterpret_cast<char*>(&secHeader), sizeof(secHeader));
		if (!file)
			break;

		auto& info       = m_sectionInfos[i];
		info.matKey      = secHeader.matKey;
		info.vertexCount = secHeader.vertexCount;
		info.indexCount  = secHeader.indexCount;
		memcpy(info.minBound, secHeader.minBound, sizeof(info.minBound));
		memcpy(info.maxBound, secHeader.maxBound, sizeof(info.maxBound));

		// 정점/인덱스 데이터 건너뜀
		const size_t skipBytes = sizeof(Render::VTXPOSNORMTANUV) * secHeader.vertexCount
		                       + sizeof(uint32_t)                * secHeader.indexCount;
		file.seekg(static_cast<std::streamoff>(skipBytes), std::ios::cur);
	}
}

void ModelLoadScene::DoExport()
{
	m_exportStatus = "Exporting...";
	m_sectionInfos.clear();

	ModelExporter exporter;
	std::string   error;

	bool ok = exporter.Export(m_srcPath, m_dstPath, error);
	m_exportStatus = ok ? "Done" : ("Failed: " + error);

	if (ok)
		ReadBoundsFromMymesh(m_dstPath);
}

void ModelLoadScene::DoLoad()
{
	auto& materialMgr = *GameEngine::MaterialManager::GetInstance();
	auto& modelMgr    = *GameEngine::ModelManager::GetInstance();

	// 섹션 정보 먼저 읽어 matKey 목록 확보
	ReadBoundsFromMymesh(m_dstPath);

	// 섹션별 matKey로 머티리얼 등록 (없으면 등록)
	for (const auto& info : m_sectionInfos)
	{
		if (materialMgr.Get(info.matKey))
			continue;

		auto result = materialMgr.LoadSync(info.matKey,
		                                   std::make_unique<DiceMaterial>(),
		                                   SHADER_KEY_MODEL);
		if (!result)
		{
			m_loadStatus = "Failed: material init error (" + info.matKey + ")";
			return;
		}
	}

	GameEngine::Model* model = modelMgr.LoadSync("Cube", m_dstPath);
	if (!model)
	{
		m_loadStatus = "Failed: model load error";
		return;
	}

	m_model = model;
	// m_sectionInfos는 이미 위에서 ReadBoundsFromMymesh로 채워짐

	// GameObject + ModelComponent 생성
	auto* go  = CreateGameObject("Dice");
	auto* mc  = go->AddComponent<GameEngine::ModelComponent>();
	mc->SetModel(m_model);
	m_modelGO = go;

	m_loadStatus = "OK";
}
