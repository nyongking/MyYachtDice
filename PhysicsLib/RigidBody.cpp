#include "PhysicsPch.h"
#include "RigidBody.h"

void RigidBody::SetInitialPosition(const Vec3& Pos)
{
	if (false == initPos || bodyType == BodyType::Kinematic)
	{
		initPos = true;
		positionWorld = Pos;
	}
}

void RigidBody::SetOrientation(float pitchDeg, float yawDeg, float rollDeg)
{
	if (false == initOrientation || bodyType == BodyType::Kinematic)
	{
		initOrientation = true;
		orientation = QuatFromEuler(pitchDeg, yawDeg, rollDeg);
	}
}

void RigidBody::SetOrientation(const Quat& quat)
{
	if (false == initOrientation || bodyType == BodyType::Kinematic)
	{
		initOrientation = true;
		orientation = quat;
	}
}

void RigidBody::SetBodyType(BodyType type)
{
	bodyType = type;

	if (type != BodyType::Dynamic)
	{
		invMass = 0.f;
		invInertiaLocal.MakeZero();
	}
}

void RigidBody::SetMass(float m)
{
	if (bodyType == BodyType::Dynamic)
		invMass = (m > 0.f) ? 1.f / m : 0.f;
}

void RigidBody::SetInertia(const Mat3& matInertia)
{
	invInertiaLocal = Mat3Inverse(matInertia);
	UpdateWorldInertia();
}

void RigidBody::SetRestitution(float r)
{
	restitution = r;
}

void RigidBody::SetFriction(float f)
{
	friction = f;
}

void RigidBody::SetLinearDamping(float d)
{
	linearDamping = d;
}

void RigidBody::SetAngularDamping(float d)
{
	angularDamping = d;
}

void RigidBody::ApplyForce(const Vec3& force)
{
	forceAccum += force;
}

void RigidBody::ApplyForceAtPoint(const Vec3& force, const Vec3& point)
{
	forceAccum += force;
	
	torqueAccum += Vec3Cross(point - positionWorld, force);
}

void RigidBody::ApplyImpulseAtPoint(const Vec3& impulse, const Vec3& point)
{
	linearVelocity += impulse * invMass; // dv = J / m
	angularVelocity += invInertiaWorld * Vec3Cross(point - positionWorld, impulse); // dw = L / I(inertia)
}

Vec3 RigidBody::GetVelocityAtPoint(const Vec3& point) const
{
	return linearVelocity + Vec3Cross(angularVelocity, point - positionWorld);
}

void RigidBody::UpdateWorldInertia()
{
	Mat3 r = QuatToMat3(orientation);
	invInertiaWorld = r * invInertiaLocal * Mat3Transpose(r); // r�� ȸ�� Matrix�� Transpose = inverse
}

void RigidBody::IntegrateVelocity(float dt)
{
	if (bodyType != BodyType::Dynamic)
		return;

	Vec3 a_lin = forceAccum * invMass;
	Vec3 a_ang = invInertiaWorld * torqueAccum;

	linearVelocity += a_lin * dt;
	angularVelocity += a_ang * dt;

	linearVelocity *= powf(1.f - linearDamping, dt);
	angularVelocity *= powf(1.f - angularDamping, dt);
}

void RigidBody::IntegratePosition(float dt)
{
	if (bodyType != BodyType::Dynamic)
		return;

	// 속도 클램프 — 과도한 충돌 반응으로 인한 폭발 방지
	float linSpeedSq = Vec3LengthSq(linearVelocity);
	if (linSpeedSq > maxLinearSpeed * maxLinearSpeed)
		linearVelocity *= maxLinearSpeed / sqrtf(linSpeedSq);

	float angSpeedSq = Vec3LengthSq(angularVelocity);
	if (angSpeedSq > maxAngularSpeed * maxAngularSpeed)
		angularVelocity *= maxAngularSpeed / sqrtf(angSpeedSq);

	positionWorld += linearVelocity * dt;

	orientation = QuatNormalize(orientation + Quat(angularVelocity.x, angularVelocity.y, angularVelocity.z, 0.f) * orientation * 0.5f * dt);
}

void RigidBody::ClearAccumulators()
{
	forceAccum.MakeZero();
	torqueAccum.MakeZero();
}

void RigidBody::SetPositionDirect(const Vec3& pos)
{
	positionWorld = pos;
}
