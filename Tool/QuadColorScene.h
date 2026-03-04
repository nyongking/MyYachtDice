#pragma once
#include "Scene.h"
#include "GameObject.h"

class QuadColorScene : public GameEngine::Scene
{
public:
	void Awake() override;

	GameEngine::GameObject* GetEditTarget() const { return m_editTarget; }

private:
	GameEngine::GameObject* m_editTarget = nullptr;
};
