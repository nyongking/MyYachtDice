#include "GameEnginePch.h"
#include "ModelComponent.h"

#include "RenderItem.h"
#include "RenderPipeline.h"
#include "SceneManager.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "Transform.h"
#include "Material.h"

namespace GameEngine
{
	void ModelComponent::Update(float dt)
	{
		if (!m_model || m_model->GetSectionCount() == 0)
			return;

		float4x4 world = GetOwner()->GetTransform()->GetWorldMatrix();

		const float4x4* pVP = nullptr;
		auto* cam = SceneManager::GetInstance().GetMainCamera();
		if (cam)
			pVP = cam->GetViewProj();

		for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
		{
			auto& section = m_model->GetSection(i);
			if (!section.material)
				continue;

			section.material->SetWorld(world);
			if (pVP)
				section.material->SetViewProj(*pVP);

			Render::RenderCommand cmd;
			cmd.geometry = &section.geometry;
			cmd.material = section.material;
			Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
		}
	}
}
