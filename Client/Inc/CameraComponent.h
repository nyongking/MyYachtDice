#pragma once
#include "Component.h"
#include "ViewProjMatrix.h"
#include "Transform.h"

namespace GameEngine
{
	class CameraComponent : public Component
	{
	public:
		void SetPerspective(float fovY, float aspect, float nearZ, float farZ);
		void SetOrthographic(float width, float height, float nearZ, float farZ);

		void SetLookAt(DirectX::FXMVECTOR eye,
		               DirectX::FXMVECTOR target,
		               DirectX::FXMVECTOR up);

		// 팔로우 카메라 API
		void SetFollowTarget(Transform* target,
		                     const float3& offset      = { 0.f,  5.f, -10.f },
		                     const float3& lookAtOffset = { 0.f,  0.f,   0.f });
		void ClearFollowTarget();
		void SetSmoothSpeed(float speed);   // 0 = 하드 팔로우, >0 = 지수 감쇠 Lerp

		void LateUpdate(float dt) override;

		const float4x4* GetViewProj()    const { return m_matrix.GetViewProj(); }
		const float4x4* GetView()        const { return m_matrix.GetView(); }
		const float4x4* GetProj()        const { return m_matrix.GetProj(); }
		const float4x4* GetInverseView() const { return m_matrix.GetInverseView(); }

		// 직렬화
		std::string GetTypeName()            const override { return "CameraComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef TOOL
		void OnInspectorGUI() override
		{
			bool changed = false;
			if (ImGui::Checkbox("Perspective", &m_isPerspective))
				changed = true;

			if (m_isPerspective)
			{
				changed |= ImGui::DragFloat("FOV (deg)", &m_fovYDeg, 0.5f, 1.f, 179.f);
				changed |= ImGui::DragFloat("Aspect",    &m_aspect,  0.01f, 0.1f, 10.f);
				changed |= ImGui::DragFloat("Near",      &m_nearZ,   0.001f, 0.001f, 100.f);
				changed |= ImGui::DragFloat("Far",       &m_farZ,    1.f, 1.f, 100000.f);
				if (changed)
					SetPerspective(m_fovYDeg * (DirectX::XM_PI / 180.f), m_aspect, m_nearZ, m_farZ);
			}
			else
			{
				changed |= ImGui::DragFloat("Width",  &m_orthoWidth,  1.f, 0.1f, 10000.f);
				changed |= ImGui::DragFloat("Height", &m_orthoHeight, 1.f, 0.1f, 10000.f);
				changed |= ImGui::DragFloat("Near",   &m_nearZ,       0.001f, 0.001f, 100.f);
				changed |= ImGui::DragFloat("Far",    &m_farZ,        1.f, 1.f, 100000.f);
				if (changed)
					SetOrthographic(m_orthoWidth, m_orthoHeight, m_nearZ, m_farZ);
			}
		}
#endif

	private:
		Render::ViewProjMatrix m_matrix;

		Transform* m_followTarget  = nullptr;
		float3     m_followOffset  = { 0.f,  5.f, -10.f };
		float3     m_lookAtOffset  = { 0.f,  0.f,   0.f };
		float      m_smoothSpeed   = 0.f;
		float3     m_currentEye   = { 0.f,  0.f,   0.f };

		// 직렬화를 위한 투영 파라미터 저장
		bool  m_isMainCamera  = false;
		bool  m_isPerspective = true;
		float m_fovYDeg       = 60.f;
		float m_aspect        = 16.f / 9.f;
		float m_nearZ         = 0.1f;
		float m_farZ          = 1000.f;
		float m_orthoWidth    = 10.f;
		float m_orthoHeight   = 10.f;
	};
}
