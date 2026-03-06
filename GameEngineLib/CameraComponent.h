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

	private:
		Render::ViewProjMatrix m_matrix;

		Transform* m_followTarget  = nullptr;
		float3     m_followOffset  = { 0.f,  5.f, -10.f };
		float3     m_lookAtOffset  = { 0.f,  0.f,   0.f };
		float      m_smoothSpeed   = 0.f;
		float3     m_currentEye   = { 0.f,  0.f,   0.f };
	};
}
