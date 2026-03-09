#pragma once
#include "Component.h"
#include "RenderPassBase.h"
#include "Model.h"
#include "GBufferMaterial.h"

namespace GameEngine
{
	class ModelComponent : public Component
	{
	public:
		void Update(float dt) override;

		void SetModel(Model* model)                            { m_model = model; }
		// 키 + 경로로 LoadSync 후 설정 (씬 로드 시 사용)
		void SetModel(const std::string& key, const std::string& path);
		void SetLayer(Render::RenderPassBase::Layer layer)     { m_layer = layer; }

		Model*             GetModel()     const { return m_model; }
		const std::string& GetModelKey()  const { return m_modelKey; }
		const std::string& GetModelPath() const { return m_modelPath; }

		// 직렬화
		std::string GetTypeName()            const override { return "ModelComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef TOOL
		void OnInspectorGUI() override
		{
			ImGui::Text("Key:  %s", m_modelKey.empty()  ? "(none)" : m_modelKey.c_str());
			ImGui::Text("Path: %s", m_modelPath.empty() ? "(none)" : m_modelPath.c_str());

			const char* layers[] = { "Opaque", "Transparent", "UI", "Effect" };
			int layerInt = static_cast<int>(m_layer);
			if (ImGui::Combo("Layer", &layerInt, layers, 4))
				m_layer = static_cast<Render::RenderPassBase::Layer>(layerInt);

			if (m_model && ImGui::CollapsingHeader("Materials"))
			{
				for (size_t i = 0; i < m_model->GetSectionCount(); ++i)
				{
					auto* mat = dynamic_cast<Render::GBufferMaterial*>(m_model->GetSection(i).material);
					if (!mat) continue;

					ImGui::PushID(static_cast<int>(i));
					ImGui::Text("Section %zu", i);
					float4 col = mat->GetAlbedoColor();
					float color[4] = { col.x, col.y, col.z, col.w };
					if (ImGui::ColorEdit4("Albedo", color))
						mat->SetAlbedoColor({ color[0], color[1], color[2], color[3] });
					ImGui::PopID();
					ImGui::Spacing();
				}
			}
		}
#endif

	private:
		Model*                        m_model     = nullptr;
		std::string                   m_modelKey;
		std::string                   m_modelPath;
		Render::RenderPassBase::Layer m_layer     = Render::RenderPassBase::Layer::Opaque;
	};
}
