#include "ClientPch.h"
#include "QuadComponent.h"

#include "EngineGlobal.h"
#include "GeometryManager.h"

QuadComponent::QuadComponent(std::unique_ptr<Render::Material> material)
{
	SetMaterial(std::move(material));
	SetGeometry(GameEngine::GGeometryManager->Get("Quad"));
}
