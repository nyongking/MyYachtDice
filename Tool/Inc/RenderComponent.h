#pragma once
#include "Component.h"
#include "RenderTypes.h"
#include "RenderPass.h"

namespace Render { class Geometry; class Material; }

namespace GameEngine
{
	class RenderComponent : public Component
	{
	public:
		void Update(float dt) override;

		void SetMaterial(std::unique_ptr<Render::Material> mat) { m_material = std::move(mat); }
		void SetGeometry(Render::Geometry* geo)                 { m_geometry = geo; }
		void SetLayer(Render::RenderPass::Layer layer)          { m_layer = layer; }

		Render::Material* GetMaterial() const { return m_material.get(); }

	protected:
		std::unique_ptr<Render::Material> m_material;
		Render::Geometry*                 m_geometry = nullptr;
		Render::RenderPass::Layer         m_layer    = Render::RenderPass::Layer::Opaque;
	};
}
