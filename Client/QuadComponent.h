#pragma once
#include "Component.h"
#include "Material.h"

class QuadComponent : public GameEngine::Component
{
public:
	explicit QuadComponent(std::shared_ptr<Render::Material> material);

	void Update(float dt) override;

	Render::Material* GetMaterial() const { return m_material.get(); }

private:
	std::shared_ptr<Render::Material> m_material;
};
