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
#include "GBufferMaterial.h"
#include "LightComponent.h"
#include "GameObject.h"
#include "BufferStruct.h"

static constexpr const char* SHADER_KEY_MODEL = "GBuffer";

void ModelLoadScene::Awake()
{
	// 방향광
	{
		auto* go    = CreateGameObject("DirectionalLight1");
		auto* light = go->AddComponent<GameEngine::LightComponent>();
		light->SetType(Render::LightType::Directional);
		light->SetDirection({ 0.3f, -1.f, 0.5f });
		light->SetColor({ 1.f, 1.f, 0.95f });
		light->SetIntensity(1.2f);
	}

	// 카메라
	auto* camGO = CreateGameObject("MainCamera");
	m_camera    = camGO->AddComponent<GameEngine::CameraComponent>();
	m_camera->SetPerspective(DirectX::XM_PIDIV4, 16.f / 9.f, 0.1f, 1000.f);
	m_camera->SetLookAt(
		DirectX::XMVectorSet(0.f, 0.05f, 0.15f, 1.f),
		DirectX::XMVectorSet(0.f, 0.f,   0.f,   1.f),
		DirectX::XMVectorSet(0.f, 1.f,   0.f,   0.f));
	GameEngine::SceneManager::GetInstance().SetMainCamera(m_camera);
}

void ModelLoadScene::Update(float dt)
{
	GameEngine::EditorScene::Update(dt);  // Hierarchy + Inspector

	BuildModelLoaderUI();
}

// ── Inspector 확장 ────────────────────────────────────────────────────────────

void ModelLoadScene::OnInspectorFooter(GameEngine::GameObject* selected)
{
	// Materials (ModelComponent 가진 GO 선택 시)
	if (m_model && selected->GetComponent<GameEngine::ModelComponent>())
	{
		if (ImGui::CollapsingHeader("Materials"))
		{
			for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
			{
				auto* mat = dynamic_cast<Render::GBufferMaterial*>(m_model->GetSection(i).material);
				if (!mat) continue;

				ImGui::PushID(static_cast<int>(i));
				ImGui::Text("Section %zu", i);
				float4 col = mat->GetAlbedoColor();
				float color[4] = { col.x, col.y, col.z, col.w };
				if (ImGui::ColorEdit4("Albedo", color))
					mat->SetAlbedoColor({ color[0], color[1], color[2], color[3] });
				ImGui::PopID();
				ImGui::Spacing();
			}
		}
	}
}

// ── Model Loader ──────────────────────────────────────────────────────────────

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

// ── 내부 유틸 ─────────────────────────────────────────────────────────────────


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

	ReadBoundsFromMymesh(m_dstPath);

	for (const auto& info : m_sectionInfos)
	{
		if (materialMgr.Get(info.matKey))
			continue;

		auto result = materialMgr.LoadSync(info.matKey,
		                                   std::make_unique<Render::GBufferMaterial>(),
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

	auto* go = CreateGameObject("Dice");
	go->AddComponent<GameEngine::ModelComponent>()->SetModel(m_model);

	m_loadStatus = "OK";
}
