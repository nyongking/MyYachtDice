#pragma once
#include "EditorScene.h"
#include "Model.h"
#include "CameraComponent.h"

class ModelLoadScene : public GameEngine::EditorScene
{
public:
	void Awake()          override;
	void Update(float dt) override;

protected:
	void OnInspectorFooter(GameEngine::GameObject* selected) override;

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

	void DoExport();
	void DoLoad();
	void ReadBoundsFromMymesh(const std::string& path);

private:
	char m_srcPath[256] = "./Bin/Resource/Model/Cube.obj";
	char m_dstPath[256] = "./Bin/Resource/Model/Cube.mymesh";

	std::string              m_exportStatus;
	std::string              m_loadStatus;
	std::vector<SectionInfo> m_sectionInfos;

	GameEngine::Model*           m_model  = nullptr;
	GameEngine::CameraComponent* m_camera = nullptr;
};
