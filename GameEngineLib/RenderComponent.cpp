#include "GameEnginePch.h"
#include "RenderComponent.h"

#include "RenderItem.h"
#include "RenderPipeline.h"
#include "SceneManager.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "Transform.h"
#include "Material.h"

namespace GameEngine
{
	static Render::RenderPassBase::Layer LayerFromString(const std::string& s)
	{
		if (s == "Transparent") return Render::RenderPassBase::Layer::Transparent;
		if (s == "UI")          return Render::RenderPassBase::Layer::UI;
		if (s == "Effect")      return Render::RenderPassBase::Layer::Effect;
		return Render::RenderPassBase::Layer::Opaque;
	}

	static std::string LayerToString(Render::RenderPassBase::Layer layer)
	{
		switch (layer)
		{
		case Render::RenderPassBase::Layer::Transparent: return "Transparent";
		case Render::RenderPassBase::Layer::UI:          return "UI";
		case Render::RenderPassBase::Layer::Effect:      return "Effect";
		default:                                         return "Opaque";
		}
	}

	MyJson RenderComponent::Serialize() const
	{
		MyJson j;
		j["type"]  = GetTypeName();
		j["layer"] = LayerToString(m_layer);
		return j;
	}

	void RenderComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("layer"))
			m_layer = LayerFromString(j["layer"].get<std::string>());
	}

	void RenderComponent::Update(float dt)
	{
		if (!m_material || !m_geometry)
			return;

		Render::RenderCommand cmd;
		cmd.geometry = m_geometry;
		cmd.material = m_material.get();
		cmd.world    = GetOwner()->GetTransform()->GetWorldMatrix();

		auto* cam = SceneManager::GetInstance().GetMainCamera();
		if (cam)
		{
			const float4x4* pVP = cam->GetViewProj();
			if (pVP)
				cmd.viewProj = *pVP;
		}

		Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
	}
}
