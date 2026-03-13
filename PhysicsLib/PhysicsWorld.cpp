#include "PhysicsPch.h"
#include "PhysicsWorld.h"

#include "RigidBody.h"
#include "Collider.h"

void PhysicsWorld::Step(float dt)
{
	auto* dp = m_dispatcher != nullptr ? m_dispatcher : &m_defaultDispatcher;

	// step1 : 중력 적용 + 속도 적분 (위치는 아직 갱신하지 않음)
	dp->ParallelFor(0, static_cast<int>(m_bodies.size()), [&](int i)
		{
			RigidBody* body = m_bodies[i];

			if (body->IsDynamic())
				body->ApplyForce(m_gravity * (1.f / body->GetInvMass()));

			body->IntegrateVelocity(dt);
			body->UpdateWorldInertia();
		});

	m_pairs.clear();
	m_collisions.clear();
	m_triggers.clear();

	// step2 : 충돌 감지 (Broadphase + Narrowphase)
	m_broadPhase.GetPairsBruteForce(m_colliders, m_pairs);

	for (auto& pair : m_pairs)
		m_narrowPhase.TestPair(pair, m_collisions, m_triggers);

	// step3 : 충돌 해결 (속도 임펄스 + 위치 보정)
	if (!m_collisions.empty())
	{
		std::vector<RigidBody*> bodyTemps;
		bodyTemps.reserve(m_bodies.size());

		for (auto& body : m_bodies)
			bodyTemps.push_back(body);

		m_islandBuilder.Build(bodyTemps, m_collisions);

		const std::vector<Island>& islands = m_islandBuilder.GetIslands();

		dp->ParallelFor(0, static_cast<int>(islands.size()), [&](int i)
			{
				const Island& land = islands[i];
				if (land.contacts.empty())
					return;

				std::vector<Contact> islandContacts;
				islandContacts.reserve(land.contacts.size());

				for (auto* ct : land.contacts)
					islandContacts.push_back(*ct);

				ContactSolver solver;
				solver.Resolve(islandContacts, dt);
			});
	}

	// step4 : 솔버가 보정한 속도로 위치/회전 갱신
	dp->ParallelFor(0, static_cast<int>(m_bodies.size()), [&](int i)
		{
			m_bodies[i]->IntegratePosition(dt);
		});

	// step5 : 클리어
	dp->ParallelFor(0, (int)m_bodies.size(), [&](int i)
		{
			m_bodies[i]->ClearAccumulators();
		});

}

void PhysicsWorld::AddRigidBody(RigidBody* body)
{
	m_bodies.push_back(body);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* body)
{
	for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it)
	{
		if (*it == body)
		{
			m_bodies.erase(it);
			return;
		}
	}
}

void PhysicsWorld::ClearRigidBody()
{
	m_bodies.clear();
}

void PhysicsWorld::AddCollider(Collider* collider)
{
	m_colliders.push_back(collider);
}

void PhysicsWorld::RemoveCollider(Collider* collider)
{
	for (auto it = m_colliders.begin(); it != m_colliders.end(); ++it)
	{
		if (*it == collider)
		{
			m_colliders.erase(it);
			return;
		}
	}
}

void PhysicsWorld::ClearColliders()
{
	m_colliders.clear();
}

void PhysicsWorld::SetDispatcher(IJobDispatcher* dispatcher)
{
	m_dispatcher = dispatcher;
}
