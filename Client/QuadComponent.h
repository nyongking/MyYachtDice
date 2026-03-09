#pragma once
#include "RenderComponent.h"

class QuadComponent : public GameEngine::RenderComponent
{
public:
	explicit QuadComponent(std::unique_ptr<Render::Material> material);
};
