#pragma once
#include "RenderComponent.h"
#include "Material.h"

namespace GameEngine
{
	class QuadComponent : public RenderComponent
	{
	public:
		void Awake() override;
		void Update(float dt) override;

		Render::Material* GetMaterial() const { return m_material.get(); }
	};


}

