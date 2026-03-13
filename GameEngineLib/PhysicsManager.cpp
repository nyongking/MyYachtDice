#include "GameEnginePch.h"
#include "PhysicsManager.h"

namespace GameEngine
{
	bool PhysicsManager::Initialize()
	{
		return true;
	}

	void PhysicsManager::Step(float dt)
	{
		// dt가 너무 크면 상한 제한 (창 드래그 등으로 수 초 쌓이는 것 방지)
		if (dt > m_maxAccum)
			dt = m_maxAccum;

		// 누적
		m_accumulator += dt;

		// 고정 간격만큼 반복 실행
		while (m_accumulator >= m_fixedDt)
		{
			m_world.Step(m_fixedDt);
			m_accumulator -= m_fixedDt;
		}

		// 남은 m_accumulator는 다음 프레임으로 이월
	}

	void PhysicsManager::Clear()
	{
		m_world.ClearRigidBody();
		m_world.ClearColliders();
	}
}
