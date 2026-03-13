#pragma once
#include "Component.h"
#include "RenderTypes.h"
#include "RenderPassBase.h"

namespace Render { class Geometry; class Material; }

namespace GameEngine
{
	class RenderComponent : public Component
	{
	public:
		void Update(float dt) override;

		void SetMaterial(std::unique_ptr<Render::Material> mat) { m_material = std::move(mat); }
		void SetGeometry(Render::Geometry* geo)                 { m_geometry = geo; }
		void SetLayer(Render::RenderPassBase::Layer layer)      { m_layer = layer; }

		Render::Material* GetMaterial() const { return m_material.get(); }

		// 직렬화 (서브클래스가 GetTypeName을 override해야 함)
		MyJson Serialize()              const override;
		void   Deserialize(const MyJson& j)   override;

#ifdef TOOL
		void OnInspectorGUI() override
		{
			const char* layers[] = { "Opaque", "Transparent", "UI", "Effect" };
			int layerInt = static_cast<int>(m_layer);
			if (ImGui::Combo("Layer", &layerInt, layers, 4))
				m_layer = static_cast<Render::RenderPassBase::Layer>(layerInt);
		}
#endif

	protected:
		std::unique_ptr<Render::Material> m_material;
		Render::Geometry*                 m_geometry = nullptr;
		Render::RenderPassBase::Layer     m_layer    = Render::RenderPassBase::Layer::Opaque;
	};
}
