#pragma once
#include "Contact.h"
#include <vector>

class ContactSolver
{
	struct ContactConstraint
	{
		Contact* ct;

		Vec3 rA;
		Vec3 rB;

		// normal
		float effectiveMassNormal; // 1 / K_nor
		float accumulatedImpulseN;
		float bias;	 // restitution + Baumgarte position stabilization

		// tangent
		Vec3 tangent;
		float effectiveMassTangent;
		float accumulatedImpulseT;

		float mixedFriction;
		float mixedRestitution;
	};

public:

	void Resolve(std::vector<Contact>& contacts, float dt);

	void SetIteration(int n) { iterations = n; }

private:

	void Presolve(std::vector<Contact>& contacts, float dt);
	void ResolveVelocity();

	std::vector<ContactConstraint> m_constraints;

	int		iterations = 10;
	float	beta = 0.2f;  // Baumgarte stabilization (0.1f ~ 0.3f)
	float	slop = 0.005f;

};

