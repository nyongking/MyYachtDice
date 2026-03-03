#include "GameEnginePch.h"
#include "Transform.h"

namespace GameEngine
{
	void Transform::SetPosition(const float3& pos)
	{
		m_position = pos;
		m_dirty    = true;
	}

	void Transform::SetRotation(const float3& eulerDeg)
	{
		m_rotation = eulerDeg;
		m_dirty    = true;
	}

	void Transform::SetScale(const float3& scale)
	{
		m_scale = scale;
		m_dirty = true;
	}

	float4x4 Transform::GetWorldMatrix() const
	{
		if (!m_dirty)
			return m_worldMatrix;

		using namespace DirectX;

		const float toRad = XM_PI / 180.f;

		XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
		XMMATRIX R = XMMatrixRotationRollPitchYaw(
			m_rotation.x * toRad,
			m_rotation.y * toRad,
			m_rotation.z * toRad);
		XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

		XMStoreFloat4x4(&m_worldMatrix, S * R * T);
		m_dirty = false;

		return m_worldMatrix;
	}
}
