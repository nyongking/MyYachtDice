#include "PhysicsPch.h"
#include "ContactSolver.h"
#include "Collider.h"
#include "RigidBody.h"


void ContactSolver::Resolve(std::vector<Contact>& contacts, float dt)
{
	if (contacts.empty())
		return;

	Presolve(contacts, dt);

	for (int iter = 0; iter < iterations; ++iter)
	{
		ResolveVelocity();
	}
}

void ContactSolver::Presolve(std::vector<Contact>& contacts, float dt)
{
	m_constraints.resize(contacts.size());

	for (size_t i = 0; i < m_constraints.size(); ++i)
	{
		Contact& ct = contacts[i];
		ContactConstraint& cc = m_constraints[i];

		cc.ct = &ct;

		RigidBody* a = ct.bodyA;
		RigidBody* b = ct.bodyB;

		cc.rA = ct.point - a->GetPosition();
		cc.rB = ct.point - b->GetPosition();

		// Normal 방향의 유효질량 계산
		Vec3 rAxN = Vec3Cross(cc.rA, ct.normal);
		Vec3 rBxN = Vec3Cross(cc.rB, ct.normal);
		Vec3 termA = Vec3Cross(Mat3MultiplyVec3(a->GetInvInertiaWorld(), rAxN), cc.rA);
		Vec3 termB = Vec3Cross(Mat3MultiplyVec3(b->GetInvInertiaWorld(), rBxN), cc.rB);

		float kNormal = a->GetInvMass() + b->GetInvMass() + Vec3Dot(termA, ct.normal)
			+ Vec3Dot(termB, ct.normal);

		cc.effectiveMassNormal = (kNormal > 1e-6f) ? (1.f / kNormal) : 0.f;

		// 상대 속도 계산
		Vec3 vA = a->GetVelocityAtPoint(ct.point);
		Vec3 vB = b->GetVelocityAtPoint(ct.point);
		Vec3 vRelative = vB - vA;

		float closingVel = Vec3Dot(vRelative, ct.normal);

		const float restitution_threshold = -1.f;

		cc.mixedRestitution = (a->GetRestitution() + b->GetRestitution()) * 0.5f;

		// Baumgarte: 관통 깊이를 속도 바이어스로 변환 (깊이 > slop 일 때만)
		float baumgarte = (beta / dt) * fmaxf(ct.depth - slop, 0.f);
		float restitutionBias = (closingVel < restitution_threshold) ? cc.mixedRestitution * closingVel : 0.f;
		cc.bias = restitutionBias - baumgarte;

		// Tangent 방향의 유효질량 계산
		Vec3 vTangent = vRelative - ct.normal * Vec3Dot(vRelative, ct.normal);
		float lenTangent = Vec3Length(vTangent);

		if (lenTangent > 1e-6f)
			cc.tangent = vTangent / lenTangent;
		else
			cc.tangent = Vec3Zero();

		Vec3 rAxT = Vec3Cross(cc.rA, cc.tangent);
		Vec3 rBxT = Vec3Cross(cc.rB, cc.tangent);
		Vec3 termTA = Vec3Cross(Mat3MultiplyVec3(a->GetInvInertiaWorld(), rAxT), cc.rA);
		Vec3 termTB = Vec3Cross(Mat3MultiplyVec3(b->GetInvInertiaWorld(), rBxT), cc.rB);

		float kTangent = a->GetInvMass() + b->GetInvMass()
			+ Vec3Dot(termTA, cc.tangent) + Vec3Dot(termTB, cc.tangent);

		cc.effectiveMassTangent = (kTangent > 1e-6f) ? (1.f / kTangent) : 0.f;

		cc.mixedFriction = sqrtf(a->GetFriction() * b->GetFriction());

		cc.accumulatedImpulseN = 0.f;
		cc.accumulatedImpulseT = 0.f;
	}
}

void ContactSolver::ResolveVelocity()
{
	for (auto& cc : m_constraints)
	{
		RigidBody* a = cc.ct->bodyA;
		RigidBody* b = cc.ct->bodyB;

		const Vec3& normal = cc.ct->normal;

		Vec3 vA = a->GetVelocityAtPoint(cc.ct->point);
		Vec3 vB = b->GetVelocityAtPoint(cc.ct->point);
		Vec3 vRelative = vB - vA;

		// normal
		float vN = Vec3Dot(vRelative, normal);
		float lambdaN = -(vN + cc.bias) * cc.effectiveMassNormal;

		// 누적 impulse는 0 이상이 되도록
		float oldAccumN = cc.accumulatedImpulseN;
		cc.accumulatedImpulseN = fmaxf(oldAccumN + lambdaN, 0.f);
		lambdaN = cc.accumulatedImpulseN - oldAccumN;

		Vec3 impulseN = normal * lambdaN;
		a->ApplyImpulseAtPoint(-impulseN, cc.ct->point);
		b->ApplyImpulseAtPoint(impulseN, cc.ct->point);

		// tangent (friction)
		vA = a->GetVelocityAtPoint(cc.ct->point);
		vB = b->GetVelocityAtPoint(cc.ct->point);
		vRelative = vB - vA;

		float vT = Vec3Dot(vRelative, cc.tangent);
		float lambdaT = -vT * cc.effectiveMassTangent;

		float frictionlimit = cc.mixedFriction * cc.accumulatedImpulseN;
		float oldAccumT = cc.accumulatedImpulseT;
		cc.accumulatedImpulseT = fmaxf(-frictionlimit, fminf(oldAccumT + lambdaT, frictionlimit));
		lambdaT = cc.accumulatedImpulseT - oldAccumT;

		Vec3 impulseT = cc.tangent * lambdaT;
		a->ApplyImpulseAtPoint(-impulseT, cc.ct->point);
		b->ApplyImpulseAtPoint(impulseT, cc.ct->point);
	}
}
