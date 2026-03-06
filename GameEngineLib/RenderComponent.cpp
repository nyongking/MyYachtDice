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
	void RenderComponent::Update(float dt)
	{
		if (!m_material || !m_geometry)
			return;

		float4x4 world = GetOwner()->GetTransform()->GetWorldMatrix();
		m_material->SetWorld(world);

		auto* cam = SceneManager::GetInstance().GetMainCamera();
		if (cam)
		{
			const float4x4* pVP = cam->GetViewProj();
			if (pVP)
				m_material->SetViewProj(*pVP);
		}

		Render::RenderCommand cmd;

		cmd.geometry = m_geometry;
		cmd.material  = m_material.get();

		Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
	}
}
