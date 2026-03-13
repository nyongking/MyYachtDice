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

		// 직렬화
		std::string GetTypeName()            const override { return "LightComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

		void SetType(Render::LightType type)         { m_data.type      = static_cast<unsigned int>(type); }
		void SetColor(const float3& color)           { m_data.color     = color; }
		void SetDirection(const float3& dir)         { m_data.direction = dir; }
		void SetIntensity(float intensity)           { m_data.intensity = intensity; }
		void SetRange(float range)                   { m_data.range     = range; }
		void SetSpotAngles(float inner, float outer) { m_data.spotInner = inner; m_data.spotOuter = outer; }

		void OnInspectorGUI() override
		{
#ifdef TOOL
			const char* types[] = { "Directional", "Point", "Spot" };
			int typeInt = static_cast<int>(m_data.type);
			if (ImGui::Combo("Type", &typeInt, types, 3))
				m_data.type = static_cast<unsigned int>(typeInt);

			ImGui::ColorEdit3("Color",    &m_data.color.x);
			ImGui::DragFloat("Intensity", &m_data.intensity, 0.01f, 0.f, 100.f);

			if (typeInt != 0)  // Point, Spot
				ImGui::DragFloat("Range", &m_data.range, 0.1f, 0.f, 10000.f);

			if (typeInt != 1)  // Directional, Spot
				ImGui::DragFloat3("Direction", &m_data.direction.x, 0.01f);

			if (typeInt == 2)  // Spot
			{
				ImGui::DragFloat("Spot Inner", &m_data.spotInner, 0.001f, 0.f, 1.f);
				ImGui::DragFloat("Spot Outer", &m_data.spotOuter, 0.001f, 0.f, 1.f);
			}
#endif
		}

	private:
		Render::LightData m_data;
	};
}
