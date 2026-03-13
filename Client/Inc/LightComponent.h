#pragma once
#include "Component.h"
#include "LightItem.h"

namespace GameEngine
{
	class LightComponent : public Component
	{
	public:
		LightComponent()  = default;
		~LightComponent() = default;

	public:
		void LateUpdate(float dt) override;

		void SetType(Render::LightType type)         { m_data.type      = static_cast<unsigned int>(type); }
		void SetColor(const float3& color)           { m_data.color     = color; }
		void SetDirection(const float3& dir)         { m_data.direction = dir; }
		void SetIntensity(float intensity)           { m_data.intensity = intensity; }
		void SetRange(float range)                   { m_data.range     = range; }
		void SetSpotAngles(float inner, float outer) { m_data.spotInner = inner; m_data.spotOuter = outer; }

	private:
		Render::LightData m_data;
	};
}
