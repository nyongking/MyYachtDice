#pragma once
#include "Component.h"
#include "RenderPass.h"
#include "Model.h"

namespace GameEngine
{
	class ModelComponent : public Component
	{
	public:
		void Update(float dt) override;

		void SetModel(Model* model)                        { m_model = model; }
		void SetLayer(Render::RenderPass::Layer layer)     { m_layer = layer; }

		Model* GetModel() const { return m_model; }

	private:
		Model*                    m_model = nullptr;
		Render::RenderPass::Layer m_layer = Render::RenderPass::Layer::Opaque;
	};
}
