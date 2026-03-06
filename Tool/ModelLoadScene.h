#pragma once
#include "Scene.h"
#include "Model.h"
#include "CameraComponent.h"
#include "imgui.h"
#include "ImGuizmo.h"

class ModelLoadScene : public GameEngine::Scene
{
public:
	void Awake()          override;
	void Update(float dt) override;

private:
	struct SectionInfo
	{
		std::string matKey;
		uint32_t    vertexCount = 0;
		uint32_t    indexCount  = 0;
		float       minBound[3] = {};
		float       maxBound[3] = {};
	};

	void BuildModelLoaderUI();
	void BuildModelInfoUI();
	void BuildCameraUI();
	void BuildTransformUI();
	void BuildMaterialUI();

	void DoExport();
	void DoLoad();
	void ApplyCamera();
	void ReadBoundsFromMymesh(const std::string& path);

private:
	char m_srcPath[256] = "./Bin/Resource/Model/Cube.obj";
	char m_dstPath[256] = "./Bin/Resource/Model/Cube.mymesh";

	std::string              m_exportStatus;
	std::string              m_loadStatus;
	std::vector<SectionInfo> m_sectionInfos;

	GameEngine::Model*            m_model   = nullptr;
	GameEngine::CameraComponent*  m_camera  = nullptr;
	GameEngine::GameObject*       m_modelGO = nullptr;

	ImGuizmo::OPERATION m_gizmoOp   = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE      m_gizmoMode = ImGuizmo::WORLD;

	float m_eye[3]    = { 0.f, 0.05f, 0.15f };
	float m_target[3] = { 0.f, 0.f,   0.f };
};
