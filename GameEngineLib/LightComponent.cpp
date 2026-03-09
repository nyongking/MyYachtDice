#include "GameEnginePch.h"
#include "LightComponent.h"

#include "RenderPipeline.h"

namespace GameEngine
{
	void LightComponent::LateUpdate(float dt)
	{
		Render::RenderPipeline::GetInstance().SubmitLight({ m_data });
	}

	static const char* s_lightTypeNames[] = { "Directional", "Point", "Spot" };

	MyJson LightComponent::Serialize() const
	{
		MyJson j;
		j["type"]      = GetTypeName();
		j["lightType"] = s_lightTypeNames[m_data.type];
		j["color"]     = { m_data.color.x,     m_data.color.y,     m_data.color.z     };
		j["direction"] = { m_data.direction.x, m_data.direction.y, m_data.direction.z };
		j["position"]  = { m_data.position.x,  m_data.position.y,  m_data.position.z  };
		j["intensity"] = m_data.intensity;
		j["range"]     = m_data.range;
		j["spotInner"] = m_data.spotInner;
		j["spotOuter"] = m_data.spotOuter;
		return j;
	}

	void LightComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("lightType"))
		{
			std::string lt = j["lightType"].get<std::string>();
			if      (lt == "Point") m_data.type = static_cast<unsigned int>(Render::LightType::Point);
			else if (lt == "Spot")  m_data.type = static_cast<unsigned int>(Render::LightType::Spot);
			else                    m_data.type = static_cast<unsigned int>(Render::LightType::Directional);
		}
		if (j.contains("color"))
			m_data.color = { j["color"][0], j["color"][1], j["color"][2] };
		if (j.contains("direction"))
			m_data.direction = { j["direction"][0], j["direction"][1], j["direction"][2] };
		if (j.contains("position"))
			m_data.position  = { j["position"][0],  j["position"][1],  j["position"][2]  };

		if (j.contains("intensity")) m_data.intensity = j["intensity"];
		if (j.contains("range"))     m_data.range     = j["range"];
		if (j.contains("spotInner")) m_data.spotInner = j["spotInner"];
		if (j.contains("spotOuter")) m_data.spotOuter = j["spotOuter"];
	}
}
