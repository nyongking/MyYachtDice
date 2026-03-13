#pragma once

#include "PhysicsWorld.h"

namespace GameEngine
{
	class PhysicsManager
	{
	public:
		bool Initialize();

		void Step(float dt);

		// 씬 전환 시 호출 — 모든 body/collider 제거
		void Clear();

		void SetFixedTimeStep(float fixedDt) { m_fixedDt = fixedDt; }
		PhysicsWorld& GetWorld() { return m_world; }

	private:
		PhysicsWorld m_world;

		float m_fixedDt     = 1.f / 60.f;  // 고정 물리 스텝 (60Hz)
		float m_accumulator = 0.f;          // 누적 시간
		float m_maxAccum    = 0.2f;         // 최대 누적 (폭주 방지, ~12스텝)
	};
}
