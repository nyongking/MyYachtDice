#pragma once
#include "Scene.h"
#include "DefaultColorMaterial.h"

class QuadColorScene : public GameEngine::Scene
{
public:
	void Awake() override;

private:
	std::shared_ptr<Render::DefaultColorMaterial> m_material;
};
