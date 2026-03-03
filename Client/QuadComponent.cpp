#include "ClientPch.h"
#include "QuadComponent.h"

#include "RenderPipeline.h"
#include "RenderDefaultRegistry.h"
#include "ViewProjManager.h"
#include "RenderItem.h"
#include "RenderPass.h"
#include "GameObject.h"

QuadComponent::QuadComponent(std::shared_ptr<Render::Material> material)
	: m_material(std::move(material))
{
}

void QuadComponent::Update(float dt)
{
	if (!m_material)
		return;

	// 월드 행렬 설정
	Render::float4x4 world = GetOwner()->GetTransform()->GetWorldMatrix();
	m_material->SetWorld(world);

	// 뷰프로젝션 행렬 설정
	const Render::float4x4* pVP = Render::ViewProjManager::GetInstance().GetCurrentViewProj();
	if (pVP)
		m_material->SetViewProj(*pVP);

	// 렌더 커맨드 제출
	auto& reg = Render::RenderDefaultRegistry::GetInstance();
	Render::RenderCommand cmd;
	cmd.geometry = reg.GetGeometry(Render::GEO_QUAD_TEX).get();
	cmd.material  = m_material.get();
	Render::RenderPipeline::GetInstance().Submit(Render::RenderPass::Layer::Opaque, cmd);
}
