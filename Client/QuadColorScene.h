#pragma once
#include "Scene.h"

class TestScene : public GameEngine::Scene
{
public:
	void Awake() override;

private:
	//std::shared_ptr<Render::DefaultColorMaterial> m_material;
};
