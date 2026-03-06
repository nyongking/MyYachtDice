#include "GameEnginePch.h"
#include "CameraComponent.h"

namespace GameEngine
{
	void CameraComponent::SetPerspective(float fovY, float aspect, float nearZ, float farZ)
	{
		using namespace DirectX;

		XMMATRIX proj = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
		m_matrix.UpdateProj(proj);
	}

	void CameraComponent::SetOrthographic(float width, float height, float nearZ, float farZ)
	{
		using namespace DirectX;

		XMMATRIX proj = XMMatrixOrthographicLH(width, height, nearZ, farZ);
		m_matrix.UpdateProj(proj);
	}

	void CameraComponent::SetLookAt(DirectX::FXMVECTOR eye,
	                                 DirectX::FXMVECTOR target,
	                                 DirectX::FXMVECTOR up)
	{
		using namespace DirectX;
		XMMATRIX view = XMMatrixLookAtLH(eye, target, up);
		m_matrix.UpdateView(view);
	}

	void CameraComponent::SetFollowTarget(Transform* target,
	                                       const float3& offset,
	                                       const float3& lookAtOffset)
	{
		m_followTarget  = target;
		m_followOffset  = offset;
		m_lookAtOffset  = lookAtOffset;

		// 첫 프레임 튀는 현상 방지: eye를 목표 위치로 즉시 스냅
		if (target)
		{
			const float3& pos = target->GetPosition();
			m_currentEye = { pos.x + offset.x,
			                 pos.y + offset.y,
			                 pos.z + offset.z };
		}
	}

	void CameraComponent::ClearFollowTarget()
	{
		m_followTarget = nullptr;
	}

	void CameraComponent::SetSmoothSpeed(float speed)
	{
		m_smoothSpeed = speed;
	}

	void CameraComponent::LateUpdate(float dt)
	{
		if (!m_followTarget) return;

		using namespace DirectX;

		const float3& targetPos = m_followTarget->GetPosition();
		XMVECTOR targetV    = XMLoadFloat3(&targetPos);
		XMVECTOR desiredEye = XMVectorAdd(targetV, XMLoadFloat3(&m_followOffset));

		XMVECTOR eye;
		if (m_smoothSpeed <= 0.f)
		{
			eye = desiredEye;
		}
		else
		{
			// 지수 감쇠 보간 — 프레임레이트 독립
			float t = 1.f - std::exp(-m_smoothSpeed * dt);
			eye = XMVectorLerp(XMLoadFloat3(&m_currentEye), desiredEye, t);
		}
		XMStoreFloat3(&m_currentEye, eye);

		XMVECTOR lookTarget = XMVectorAdd(targetV, XMLoadFloat3(&m_lookAtOffset));
		XMVECTOR up         = XMVectorSet(0.f, 1.f, 0.f, 0.f);

		m_matrix.UpdateView(XMMatrixLookAtLH(eye, lookTarget, up));
	}
}
