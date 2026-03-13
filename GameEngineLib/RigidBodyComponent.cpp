#include "GameEnginePch.h"
#include "RigidBodyComponent.h"
#include "PhysicsConvert.h"
#include "PhysicsManager.h"
#include "EngineGlobal.h"
#include "GameObject.h"
#include "Transform.h"
#include "Operation.h"

#ifdef _DEBUG
#include "RenderItem.h"
#include "RenderPipeline.h"
#include "RenderPassBase.h"
#include "SceneManager.h"
#include "CameraComponent.h"
#include "GeometryManager.h"
#include "Material.h"
#endif

namespace GameEngine
{
	// ── Lifecycle ───────────────────────────────────

	void RigidBodyComponent::Awake()
	{
		m_body = std::make_unique<RigidBody>();

		m_body->SetBodyType(m_initBodyType);
		m_body->SetMass(m_initMass);
		m_body->SetRestitution(m_initRestitution);
		m_body->SetFriction(m_initFriction);
		m_body->SetLinearDamping(m_initLinearDamp);
		m_body->SetAngularDamping(m_initAngularDamp);
	}

	void RigidBodyComponent::Start()
	{
		SyncTransformToBody();
		RegisterToWorld();
	}

	void RigidBodyComponent::Update(float dt)
	{
		if (!m_body) return;

		if (m_body->IsDynamic())
			SyncBodyToTransform();
		else if (m_body->IsKinematic())
			SyncTransformToBody();
	}

	void RigidBodyComponent::LateUpdate(float dt)
	{
#ifdef _DEBUG
		if (!GDebugDrawColliders || !GDebugWireframeMaterial) return;
		if (m_shape == ColliderShape::None || m_colliders.empty()) return;

		auto* cam = SceneManager::GetInstance().GetMainCamera();
		if (!cam || !cam->GetViewProj()) return;

		Transform* tr = GetOwner()->GetTransform();
		bool isTrigger = m_colliders[0]->isTrigger;

		Render::Geometry* wireGeo = nullptr;
		float4x4 wireWorld;

		using namespace DirectX;

		if (m_shape == ColliderShape::Sphere)
		{
			auto* sc = static_cast<SphereCollider*>(m_colliders[0].get());
			wireGeo = GGeometryManager->Get(isTrigger ? "WireSphereTrigger" : "WireSphere");

			float r = sc->radius;
			float3 pos = tr->GetPosition();
			float3 rot = tr->GetRotation();

			XMMATRIX S = XMMatrixScaling(r, r, r);
			float toRad = XM_PI / 180.f;
			XMMATRIX R = XMMatrixRotationX(rot.x * toRad)
			           * XMMatrixRotationY(rot.y * toRad)
			           * XMMatrixRotationZ(rot.z * toRad);
			XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);

			XMStoreFloat4x4(&wireWorld, S * R * T);
		}
		else if (m_shape == ColliderShape::Box)
		{
			auto* bc = static_cast<BoxCollider*>(m_colliders[0].get());
			wireGeo = GGeometryManager->Get(isTrigger ? "WireBoxTrigger" : "WireBox");

			float3 he = ToEngine(bc->halfExtents);
			float3 pos = tr->GetPosition();
			float3 rot = tr->GetRotation();

			XMMATRIX S = XMMatrixScaling(he.x * 2.f, he.y * 2.f, he.z * 2.f);
			float toRad = XM_PI / 180.f;
			XMMATRIX R = XMMatrixRotationX(rot.x * toRad)
			           * XMMatrixRotationY(rot.y * toRad)
			           * XMMatrixRotationZ(rot.z * toRad);
			XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);

			XMStoreFloat4x4(&wireWorld, S * R * T);
		}
		else if (m_shape == ColliderShape::Plane)
		{
			wireGeo = GGeometryManager->Get(isTrigger ? "WirePlaneTrigger" : "WirePlane");

			float3 pos = tr->GetPosition();
			float3 rot = tr->GetRotation();

			float toRad = XM_PI / 180.f;
			XMMATRIX R = XMMatrixRotationX(rot.x * toRad)
			           * XMMatrixRotationY(rot.y * toRad)
			           * XMMatrixRotationZ(rot.z * toRad);
			XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);

			XMStoreFloat4x4(&wireWorld, R * T);
		}

		if (!wireGeo) return;

		Render::RenderCommand cmd;
		cmd.geometry = wireGeo;
		cmd.material = GDebugWireframeMaterial;
		cmd.world    = wireWorld;
		cmd.viewProj = *cam->GetViewProj();

		Render::RenderPipeline::GetInstance().Submit(
			Render::RenderPassBase::Layer::Effect, cmd);
#endif
	}

	void RigidBodyComponent::OnDestroy()
	{
		UnregisterFromWorld();
		m_colliders.clear();
		m_body.reset();
	}

	void RigidBodyComponent::UpdateInertia()
	{
		if (!m_body || !m_body->IsDynamic() || m_body->GetInvMass() <= 0.f)
			return;
		if (m_colliders.empty())
			return;

		float mass = 1.f / m_body->GetInvMass();
		Mat3 inertia;
		inertia.MakeZero();

		if (m_shape == ColliderShape::Sphere)
		{
			auto* sc = static_cast<SphereCollider*>(m_colliders[0].get());
			float val = (2.f / 5.f) * mass * sc->radius * sc->radius;
			inertia.m[0][0] = val;
			inertia.m[1][1] = val;
			inertia.m[2][2] = val;
		}
		else if (m_shape == ColliderShape::Box)
		{
			auto* bc = static_cast<BoxCollider*>(m_colliders[0].get());
			float wx = 2.f * bc->halfExtents.x;
			float hy = 2.f * bc->halfExtents.y;
			float dz = 2.f * bc->halfExtents.z;
			float k = mass / 12.f;
			inertia.m[0][0] = k * (hy * hy + dz * dz);
			inertia.m[1][1] = k * (wx * wx + dz * dz);
			inertia.m[2][2] = k * (wx * wx + hy * hy);
		}
		else
		{
			return;
		}

		m_body->SetInertia(inertia);
	}

	// ── Transform 동기화 ────────────────────────────

	void RigidBodyComponent::SyncTransformToBody()
	{
		if (!m_body) return;

		Transform* tr = GetOwner()->GetTransform();
		m_body->SetInitialPosition(ToPhysics(tr->GetPosition()));
		m_body->SetOrientation(EulerToPhysicsQuat(tr->GetRotation()));
	}

	void RigidBodyComponent::SyncBodyToTransform()
	{
		if (!m_body) return;

		Transform* tr = GetOwner()->GetTransform();
		tr->SetPosition(ToEngine(m_body->GetPosition()));
		tr->SetRotation(PhysicsQuatToEuler(m_body->GetOrientation()));
	}

	// ── PhysicsWorld 등록/해제 ──────────────────────

	void RigidBodyComponent::RegisterToWorld()
	{
		if (m_registered || !GPhysicsManager) return;

		PhysicsWorld& world = GPhysicsManager->GetWorld();

		if (m_body)
			world.AddRigidBody(m_body.get());

		for (auto& col : m_colliders)
			world.AddCollider(col.get());

		m_registered = true;
	}

	void RigidBodyComponent::UnregisterFromWorld()
	{
		if (!m_registered || !GPhysicsManager) return;

		PhysicsWorld& world = GPhysicsManager->GetWorld();

		for (auto& col : m_colliders)
			world.RemoveCollider(col.get());

		if (m_body)
			world.RemoveRigidBody(m_body.get());

		m_registered = false;
	}

	void RigidBodyComponent::ReplaceCollider(std::unique_ptr<Collider> newCol)
	{
		// 기존 collider 제거
		if (m_registered && GPhysicsManager)
		{
			for (auto& col : m_colliders)
				GPhysicsManager->GetWorld().RemoveCollider(col.get());
		}
		m_colliders.clear();

		// 새 collider 설정
		if (newCol)
		{
			newCol->body  = m_body.get();
			newCol->owner = this;

			if (m_registered && GPhysicsManager)
				GPhysicsManager->GetWorld().AddCollider(newCol.get());

			m_colliders.push_back(std::move(newCol));
		}
	}

	// ── Body 설정 ───────────────────────────────────

	void RigidBodyComponent::SetBodyType(RigidBody::BodyType type)
	{
		m_initBodyType = type;
		if (m_body) m_body->SetBodyType(type);
	}

	void RigidBodyComponent::SetMass(float mass)
	{
		m_initMass = mass;
		if (m_body) m_body->SetMass(mass);
		UpdateInertia();
	}

	void RigidBodyComponent::SetRestitution(float r)
	{
		m_initRestitution = r;
		if (m_body) m_body->SetRestitution(r);
	}

	void RigidBodyComponent::SetFriction(float f)
	{
		m_initFriction = f;
		if (m_body) m_body->SetFriction(f);
	}

	void RigidBodyComponent::SetLinearDamping(float d)
	{
		m_initLinearDamp = d;
		if (m_body) m_body->SetLinearDamping(d);
	}

	void RigidBodyComponent::SetAngularDamping(float d)
	{
		m_initAngularDamp = d;
		if (m_body) m_body->SetAngularDamping(d);
	}

	// ── Collider 설정 ──────────────────────────────

	void RigidBodyComponent::SetSphereCollider(float radius)
	{
		auto col    = std::make_unique<SphereCollider>();
		col->radius = radius;
		m_shape     = ColliderShape::Sphere;
		ReplaceCollider(std::move(col));
		UpdateInertia();
	}

	void RigidBodyComponent::SetBoxCollider(const float3& halfExtents)
	{
		auto col         = std::make_unique<BoxCollider>();
		col->halfExtents = ToPhysics(halfExtents);
		m_shape          = ColliderShape::Box;
		ReplaceCollider(std::move(col));
		UpdateInertia();
	}

	void RigidBodyComponent::SetPlaneCollider(const float3& normal, float dist)
	{
		auto col    = std::make_unique<PlaneCollider>();
		col->normal = ToPhysics(normal);
		col->dist   = dist;
		m_shape     = ColliderShape::Plane;
		ReplaceCollider(std::move(col));
	}

	void RigidBodyComponent::SetTrigger(bool trigger)
	{
		for (auto& col : m_colliders)
			col->isTrigger = trigger;
	}

	void RigidBodyComponent::SetCollisionLayer(unsigned int layer, unsigned int mask)
	{
		for (auto& col : m_colliders)
		{
			col->layer     = layer;
			col->layerMask = mask;
		}
	}

	// ── Force API ──────────────────────────────────

	void RigidBodyComponent::ApplyForce(const float3& force)
	{
		if (m_body && m_body->IsDynamic())
			m_body->ApplyForce(ToPhysics(force));
	}

	void RigidBodyComponent::ApplyForceAtPoint(const float3& force, const float3& point)
	{
		if (m_body && m_body->IsDynamic())
			m_body->ApplyForceAtPoint(ToPhysics(force), ToPhysics(point));
	}

	void RigidBodyComponent::ApplyImpulse(const float3& impulse, const float3& point)
	{
		if (m_body && m_body->IsDynamic())
			m_body->ApplyImpulseAtPoint(ToPhysics(impulse), ToPhysics(point));
	}

	// ── 직렬화 ─────────────────────────────────────

	static const char* s_bodyTypeNames[] = { "Static", "Kinematic", "Dynamic" };
	static const char* s_shapeNames[]    = { "None", "Sphere", "Box", "Plane" };

	MyJson RigidBodyComponent::Serialize() const
	{
		MyJson j;
		j["type"]          = GetTypeName();
		j["bodyType"]      = s_bodyTypeNames[static_cast<int>(m_initBodyType)];
		j["mass"]          = m_initMass;
		j["restitution"]   = m_initRestitution;
		j["friction"]      = m_initFriction;
		j["linearDamping"] = m_initLinearDamp;
		j["angularDamping"]= m_initAngularDamp;

		MyJson colJson;
		colJson["shape"] = s_shapeNames[static_cast<int>(m_shape)];

		if (m_shape == ColliderShape::Sphere && !m_colliders.empty())
		{
			auto* sc = static_cast<SphereCollider*>(m_colliders[0].get());
			colJson["radius"] = sc->radius;
		}
		else if (m_shape == ColliderShape::Box && !m_colliders.empty())
		{
			auto* bc = static_cast<BoxCollider*>(m_colliders[0].get());
			colJson["halfExtents"] = { bc->halfExtents.x, bc->halfExtents.y, bc->halfExtents.z };
		}
		else if (m_shape == ColliderShape::Plane && !m_colliders.empty())
		{
			auto* pc = static_cast<PlaneCollider*>(m_colliders[0].get());
			colJson["normal"] = { pc->normal.x, pc->normal.y, pc->normal.z };
			colJson["dist"]   = pc->dist;
		}

		if (!m_colliders.empty())
		{
			colJson["isTrigger"] = m_colliders[0]->isTrigger;
			colJson["layer"]     = m_colliders[0]->layer;
			colJson["layerMask"] = m_colliders[0]->layerMask;
		}

		j["collider"] = colJson;
		return j;
	}

	void RigidBodyComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("bodyType"))
		{
			std::string bt = j["bodyType"].get<std::string>();
			if      (bt == "Static")    m_initBodyType = RigidBody::BodyType::Static;
			else if (bt == "Kinematic") m_initBodyType = RigidBody::BodyType::Kinematic;
			else                        m_initBodyType = RigidBody::BodyType::Dynamic;
		}

		if (j.contains("mass"))          m_initMass        = j["mass"];
		if (j.contains("restitution"))   m_initRestitution = j["restitution"];
		if (j.contains("friction"))      m_initFriction    = j["friction"];
		if (j.contains("linearDamping")) m_initLinearDamp  = j["linearDamping"];
		if (j.contains("angularDamping"))m_initAngularDamp = j["angularDamping"];

		// body가 이미 생성된 상태(Awake 이후)면 즉시 적용
		if (m_body)
		{
			m_body->SetBodyType(m_initBodyType);
			m_body->SetMass(m_initMass);
			m_body->SetRestitution(m_initRestitution);
			m_body->SetFriction(m_initFriction);
			m_body->SetLinearDamping(m_initLinearDamp);
			m_body->SetAngularDamping(m_initAngularDamp);
		}

		if (j.contains("collider"))
		{
			const MyJson& cj = j["collider"];
			std::string shape = cj.value("shape", "None");

			if (shape == "Sphere")
			{
				float r = cj.value("radius", 0.5f);
				SetSphereCollider(r);
			}
			else if (shape == "Box")
			{
				float3 he = { 0.5f, 0.5f, 0.5f };
				if (cj.contains("halfExtents"))
					he = { cj["halfExtents"][0], cj["halfExtents"][1], cj["halfExtents"][2] };
				SetBoxCollider(he);
			}
			else if (shape == "Plane")
			{
				float3 n = { 0.f, 1.f, 0.f };
				float  d = 0.f;
				if (cj.contains("normal"))
					n = { cj["normal"][0], cj["normal"][1], cj["normal"][2] };
				if (cj.contains("dist"))
					d = cj["dist"];
				SetPlaneCollider(n, d);
			}

			if (!m_colliders.empty())
			{
				if (cj.contains("isTrigger")) m_colliders[0]->isTrigger = cj["isTrigger"];
				if (cj.contains("layer"))     m_colliders[0]->layer     = cj["layer"];
				if (cj.contains("layerMask")) m_colliders[0]->layerMask = cj["layerMask"];
			}
		}
	}

}
