#include "GameEnginePch.h"
#include "CameraComponent.h"
#include "SceneManager.h"

namespace GameEngine
{
	void CameraComponent::SetPerspective(float fovY, float aspect, float nearZ, float farZ)
	{
		using namespace DirectX;

		m_isPerspective = true;
		m_fovYDeg       = fovY * (180.f / XM_PI);
		m_aspect        = aspect;
		m_nearZ         = nearZ;
		m_farZ          = farZ;

		XMMATRIX proj = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
		m_matrix.UpdateProj(proj);
	}

	void CameraComponent::SetOrthographic(float width, float height, float nearZ, float farZ)
	{
		using namespace DirectX;

		m_isPerspective = false;
		m_orthoWidth    = width;
		m_orthoHeight   = height;
		m_nearZ         = nearZ;
		m_farZ          = farZ;

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

	MyJson CameraComponent::Serialize() const
	{
		MyJson j;
		j["type"]          = GetTypeName();
		j["isMainCamera"]  = m_isMainCamera;
		j["isPerspective"] = m_isPerspective;
		j["nearZ"]         = m_nearZ;
		j["farZ"]          = m_farZ;
		j["eye"]           = { m_eye.x,    m_eye.y,    m_eye.z    };
		j["target"]        = { m_target.x, m_target.y, m_target.z };

		if (m_isPerspective)
		{
			j["fovYDeg"] = m_fovYDeg;
			j["aspect"]  = m_aspect;
		}
		else
		{
			j["orthoWidth"]  = m_orthoWidth;
			j["orthoHeight"] = m_orthoHeight;
		}

		return j;
	}

	void CameraComponent::Deserialize(const MyJson& j)
	{
		using namespace DirectX;

		bool  isPerspective = j.value("isPerspective", true);
		float nearZ         = j.value("nearZ",  0.1f);
		float farZ          = j.value("farZ",  1000.f);

		if (isPerspective)
		{
			float fovYDeg = j.value("fovYDeg", 60.f);
			float aspect  = j.value("aspect",  16.f / 9.f);
			SetPerspective(fovYDeg * (XM_PI / 180.f), aspect, nearZ, farZ);
		}
		else
		{
			float w = j.value("orthoWidth",  10.f);
			float h = j.value("orthoHeight", 10.f);
			SetOrthographic(w, h, nearZ, farZ);
		}

		if (j.contains("eye"))
			m_eye    = { j["eye"][0],    j["eye"][1],    j["eye"][2]    };
		if (j.contains("target"))
			m_target = { j["target"][0], j["target"][1], j["target"][2] };

		// View matrix 초기화 (SetLookAt 없이 방치 시 행렬이 0으로 남음)
		SetLookAt(
			XMVectorSet(m_eye.x,    m_eye.y,    m_eye.z,    1.f),
			XMVectorSet(m_target.x, m_target.y, m_target.z, 1.f),
			XMVectorSet(0.f, 1.f, 0.f, 0.f));

		m_isMainCamera = j.value("isMainCamera", false);
		if (m_isMainCamera)
			SceneManager::GetInstance().SetMainCamera(this);
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
