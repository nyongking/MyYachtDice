#include "ClientPch.h"
#include "QuadColorScene.h"
#include "QuadComponent.h"
#include "RenderDefaultRegistry.h"

void QuadColorScene::Awake()
{
	auto& reg = Render::RenderDefaultRegistry::GetInstance();

	m_material = std::make_shared<Render::DefaultColorMaterial>();
	m_material->Initialize(reg.GetShaderGroup(Render::SHADER_QUAD_TEX).get());
	m_material->SetColor({ 1.f, 0.f, 0.f, 1.f }); // 빨간색

	auto* go = CreateGameObject("Quad");
	go->AddComponent<QuadComponent>(m_material);
}
