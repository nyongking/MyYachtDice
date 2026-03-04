#pragma once
#include "RenderComponent.h"

class QuadComponent : public GameEngine::RenderComponent
{
public:
	explicit QuadComponent(std::shared_ptr<Render::Material> material);
};
