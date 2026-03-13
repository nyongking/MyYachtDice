#pragma once
#include "Vec3.h"
#include "Quat.h"
#include "Mat3.h"

class RigidBody
{
public:
	enum class BodyType { Static, Kinematic, Dynamic };
	// Setter----------------------------------
	void		SetInitialPosition(const Vec3& Pos);
	void		SetOrientation(float pitchDeg, float yawDeg, float rollDeg);
	void		SetOrientation(const Quat& orientation);

	void		SetBodyType(BodyType type);
	void		SetMass(float mass);
	void		SetInertia(const Mat3& matInertia);
	void		SetRestitution(float r);
	void		SetFriction(float f);
	void		SetLinearDamping(float d);
	void		SetAngularDamping(float d);

	// Getter----------------------------------
	BodyType	GetBodyType() const { return bodyType; }
	Vec3		GetPosition() const { return positionWorld; }
	Quat		GetOrientation() const { return orientation; }

	Vec3		GetLinearVelocity() const { return linearVelocity; }
	Vec3		GetAngularVelocity() const { return angularVelocity; }

	float		GetInvMass() const { return invMass; }
	Mat3		GetInvInertiaWorld() const { return invInertiaWorld; }
	float		GetRestitution() const { return restitution; }
	float		GetFriction() const { return friction; }

	bool		IsStatic() const { return bodyType == BodyType::Static; }
	bool		IsKinematic() const { return bodyType == BodyType::Kinematic; }
	bool		IsDynamic() const { return bodyType == BodyType::Dynamic; }

	// ----------------------------------
	// 
	// 질량 중심에 힘 적용 ->  forceAccum 변경
	void		ApplyForce(const Vec3& force);

	// 특정 지점에 point 적용 -> forceAccum, torqueAccum 변경
	void		ApplyForceAtPoint(const Vec3& force, const Vec3& point); 

	// 특정 지점에 충격량(J = F * t) 적용, -> linearVelocity, angularVelocity 변경 (J = m * dV)
	void		ApplyImpulseAtPoint(const Vec3& impulse, const Vec3& point);

	// 특정 지점의 속도 구하기
	Vec3		GetVelocityAtPoint(const Vec3& point) const;

	// 회전 상황을 기준으로 Mass Inertia 업데이트,
	void		UpdateWorldInertia();

	// 적분 (Semi-implicit Euler: 속도 먼저, 위치 나중에)
	void		IntegrateVelocity(float dt);
	void		IntegratePosition(float dt);

	// accum = 0
	void		ClearAccumulators();

private:
	friend class ContactSolver;
	void		SetPositionDirect(const Vec3& pos);

private:
	bool		initPos = false;
	bool		initOrientation = false;

	BodyType	bodyType = BodyType::Static;
	Vec3		positionWorld;
	Quat		orientation;

	Vec3		linearVelocity;
	Vec3		angularVelocity;

	float		invMass = 0.f;
	Mat3		invInertiaLocal;
	Mat3		invInertiaWorld;

	float		restitution = 0.5f;
	float		friction = 0.5f;

	float		linearDamping = 0.01f;
	float		angularDamping = 0.01f;

	float		maxLinearSpeed = 50.f;
	float		maxAngularSpeed = 50.f;

	Vec3		forceAccum;
	Vec3		torqueAccum;
};

