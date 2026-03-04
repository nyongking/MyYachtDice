#include "ClientPch.h"
#include "QuadComponent.h"

#include "RenderDefaultRegistry.h"

QuadComponent::QuadComponent(std::shared_ptr<Render::Material> material)
{
	SetMaterial(std::move(material));
	SetGeometry(Render::RenderDefaultRegistry::GetInstance().GetGeometry(Render::GEO_QUAD_TEX).get());
}
