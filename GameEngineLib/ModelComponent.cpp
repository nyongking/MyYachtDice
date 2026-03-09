#include "GameEnginePch.h"
#include "ModelComponent.h"

#include "RenderItem.h"
#include "RenderPipeline.h"
#include "SceneManager.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "Transform.h"
#include "Material.h"
#include "ModelManager.h"
#include "EngineGlobal.h"
#include "GBufferMaterial.h"

namespace GameEngine
{
	static Render::RenderPassBase::Layer LayerFromString(const std::string& s)
	{
		if (s == "Transparent") return Render::RenderPassBase::Layer::Transparent;
		if (s == "UI")          return Render::RenderPassBase::Layer::UI;
		if (s == "Effect")      return Render::RenderPassBase::Layer::Effect;
		return Render::RenderPassBase::Layer::Opaque;
	}

	static std::string LayerToString(Render::RenderPassBase::Layer layer)
	{
		switch (layer)
		{
		case Render::RenderPassBase::Layer::Transparent: return "Transparent";
		case Render::RenderPassBase::Layer::UI:          return "UI";
		case Render::RenderPassBase::Layer::Effect:      return "Effect";
		default:                                         return "Opaque";
		}
	}

	void ModelComponent::SetModel(const std::string& key, const std::string& path)
	{
		m_modelKey  = key;
		m_modelPath = path;

		if (!GModelManager) return;

		// 캐시 히트면 바로 반환, 없으면 LoadSync로 로드
		m_model = GModelManager->Get(key);
		if (!m_model)
			m_model = GModelManager->LoadSync(key, path);
	}

	MyJson ModelComponent::Serialize() const
	{
		MyJson j;
		j["type"]      = GetTypeName();
		j["modelKey"]  = m_modelKey;
		j["modelPath"] = m_modelPath;
		j["layer"]     = LayerToString(m_layer);

		if (m_model)
		{
			MyJson sections = MyJson::array();
			for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
			{
				MyJson sec;
				auto* mat = dynamic_cast<Render::GBufferMaterial*>(m_model->GetSection(i).material);
				if (mat)
				{
					float4 col = mat->GetAlbedoColor();
					sec["albedoColor"] = { col.x, col.y, col.z, col.w };
				}
				sections.push_back(sec);
			}
			j["sections"] = sections;
		}

		return j;
	}

	void ModelComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("modelKey") && j.contains("modelPath"))
			SetModel(j["modelKey"].get<std::string>(), j["modelPath"].get<std::string>());

		if (j.contains("layer"))
			m_layer = LayerFromString(j["layer"].get<std::string>());

		// 모델 로드 후 section별 색상 복원
		if (m_model && j.contains("sections"))
		{
			const auto& sections = j["sections"];
			for (size_t i = 0; i < m_model->GetSectionCount() && i < sections.size(); ++i)
			{
				const auto& sec = sections[i];
				if (!sec.contains("albedoColor")) continue;

				auto* mat = dynamic_cast<Render::GBufferMaterial*>(m_model->GetSection(i).material);
				if (!mat) continue;

				const auto& c = sec["albedoColor"];
				mat->SetAlbedoColor({ c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>() });
			}
		}
	}

	void ModelComponent::Update(float dt)
	{
		if (!m_model || m_model->GetSectionCount() == 0)
			return;

		float4x4 world = GetOwner()->GetTransform()->GetWorldMatrix();

		float4x4 viewProj = {};
		auto* cam = SceneManager::GetInstance().GetMainCamera();
		if (cam)
		{
			const float4x4* pVP = cam->GetViewProj();
			if (pVP)
				viewProj = *pVP;
		}

		for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
		{
			auto& section = m_model->GetSection(i);
			if (!section.material)
				continue;

			Render::RenderCommand cmd;
			cmd.geometry = &section.geometry;
			cmd.material = section.material;
			cmd.world    = world;
			cmd.viewProj = viewProj;
			Render::RenderPipeline::GetInstance().Submit(m_layer, cmd);
		}
	}
}
