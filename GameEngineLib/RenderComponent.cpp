#include "GameEnginePch.h"
#include "RenderComponent.h"

#include "RenderItem.h"
#include "RenderPipeline.h"
#include "ViewProjManager.h"
#include "GameObject.h"
#include "Transform.h"
#include "Material.h"

namespace GameEngine
{
	void RenderComponent::Update(float dt)
	{
		if (!m_material || !m_geometry)
			return;

		float4x4 world = GetOwner()->GetTransform()->GetWorldMatrix();
		m_material->SetWorld(world);

		const float4x4* pVP = Render::ViewProjManager::GetInstance().GetCurrentViewProj();
		if (pVP)
			m_material->SetViewProj(*pVP);

		Render::RenderCommand cmd;

		cmd.geometry = m_geometry;
		cmd.material  = m_material.get();

		Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
	}
}
