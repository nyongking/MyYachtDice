#pragma once
#include "Scene.h"

class QuadColorScene : public GameEngine::Scene
{
public:
	void Awake() override;

private:
	//std::shared_ptr<Render::DefaultColorMaterial> m_material;
};
