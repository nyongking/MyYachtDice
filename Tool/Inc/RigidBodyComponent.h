#pragma once

#include "Component.h"
#include "RigidBody.h"
#include "Collider.h"

#ifdef TOOL
#include "imgui.h"
#endif

namespace GameEngine
{
	class RigidBodyComponent : public Component
	{
	public:
		enum class ColliderShape { None, Sphere, Box, Plane };

		RigidBodyComponent()  = default;
		~RigidBodyComponent() = default;

	public:
		void Awake()              override;
		void Start()              override;
		void Update(float dt)     override;
		void LateUpdate(float dt) override;
		void OnDestroy()          override;

		// 직렬화
		std::string GetTypeName()            const override { return "RigidBodyComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

		// ── Body 설정 ──
		void SetBodyType(RigidBody::BodyType type);
		void SetMass(float mass);
		void SetRestitution(float r);
		void SetFriction(float f);
		void SetLinearDamping(float d);
		void SetAngularDamping(float d);

		// ── Collider 설정 ──
		void SetSphereCollider(float radius);
		void SetBoxCollider(const float3& halfExtents);
		void SetPlaneCollider(const float3& normal, float dist);
		void SetTrigger(bool trigger);
		void SetCollisionLayer(unsigned int layer, unsigned int mask);

		// ── Force API (Dynamic body 전용) ──
		void ApplyForce(const float3& force);
		void ApplyForceAtPoint(const float3& force, const float3& point);
		void ApplyImpulse(const float3& impulse, const float3& point);

		// ── 조회 ──
		RigidBody*    GetRigidBody() const { return m_body.get(); }
		Collider*     GetCollider()  const { return m_colliders.empty() ? nullptr : m_colliders[0].get(); }
		ColliderShape GetColliderShape() const { return m_shape; }

		void OnInspectorGUI() override
		{
#ifdef TOOL
			// Body Type
			const char* bodyTypes[] = { "Static", "Kinematic", "Dynamic" };
			int btIdx = static_cast<int>(m_initBodyType);
			if (ImGui::Combo("Body Type", &btIdx, bodyTypes, 3))
				SetBodyType(static_cast<RigidBody::BodyType>(btIdx));

			if (m_initBodyType == RigidBody::BodyType::Dynamic)
			{
				if (ImGui::DragFloat("Mass", &m_initMass, 0.1f, 0.01f, 10000.f)) SetMass(m_initMass);
			}

			if (ImGui::DragFloat("Restitution",    &m_initRestitution, 0.01f, 0.f, 1.f))   SetRestitution(m_initRestitution);
			if (ImGui::DragFloat("Friction",        &m_initFriction,    0.01f, 0.f, 2.f))   SetFriction(m_initFriction);
			if (ImGui::DragFloat("Linear Damping",  &m_initLinearDamp,  0.001f, 0.f, 1.f))  SetLinearDamping(m_initLinearDamp);
			if (ImGui::DragFloat("Angular Damping", &m_initAngularDamp, 0.001f, 0.f, 1.f))  SetAngularDamping(m_initAngularDamp);

			ImGui::Separator();
			ImGui::Text("Collider");

			const char* shapes[] = { "None", "Sphere", "Box", "Plane" };
			int shapeIdx = static_cast<int>(m_shape);
			if (ImGui::Combo("Shape", &shapeIdx, shapes, 4))
			{
				ColliderShape newShape = static_cast<ColliderShape>(shapeIdx);
				if (newShape != m_shape)
				{
					switch (newShape)
					{
					case ColliderShape::Sphere: SetSphereCollider(0.5f);                   break;
					case ColliderShape::Box:    SetBoxCollider({ 0.5f, 0.5f, 0.5f });      break;
					case ColliderShape::Plane:  SetPlaneCollider({ 0.f, 1.f, 0.f }, 0.f);  break;
					default:                    ReplaceCollider(nullptr); m_shape = ColliderShape::None; break;
					}
				}
			}

			if (m_shape == ColliderShape::Sphere && !m_colliders.empty())
			{
				auto* sc = static_cast<SphereCollider*>(m_colliders[0].get());
				ImGui::DragFloat("Radius", &sc->radius, 0.01f, 0.01f, 1000.f);
			}
			else if (m_shape == ColliderShape::Box && !m_colliders.empty())
			{
				auto* bc = static_cast<BoxCollider*>(m_colliders[0].get());
				ImGui::DragFloat3("Half Extents", &bc->halfExtents.x, 0.01f, 0.01f, 1000.f);
			}
			else if (m_shape == ColliderShape::Plane && !m_colliders.empty())
			{
				auto* pc = static_cast<PlaneCollider*>(m_colliders[0].get());
				ImGui::DragFloat3("Normal", &pc->normal.x, 0.01f, -1.f, 1.f);
				ImGui::DragFloat("Distance", &pc->dist, 0.1f);
			}

			if (!m_colliders.empty())
			{
				bool trigger = m_colliders[0]->isTrigger;
				if (ImGui::Checkbox("Is Trigger", &trigger))
					SetTrigger(trigger);
			}
#endif
		}

	private:
		void SyncTransformToBody();   // Transform → RigidBody (초기화 / Kinematic)
		void SyncBodyToTransform();   // RigidBody → Transform (Dynamic 시뮬레이션 결과)

		void RegisterToWorld();
		void UnregisterFromWorld();
		void ReplaceCollider(std::unique_ptr<Collider> newCol);
		void UpdateInertia();         // collider 모양 + mass 기반 관성 텐서 재계산

	private:
		std::unique_ptr<RigidBody>                m_body;
		std::vector<std::unique_ptr<Collider>>    m_colliders;
		ColliderShape                             m_shape      = ColliderShape::None;
		bool                                      m_registered = false;

		// 직렬화용 캐시 (Deserialize 후 Awake/Start에서 적용)
		RigidBody::BodyType m_initBodyType    = RigidBody::BodyType::Dynamic;
		float               m_initMass        = 1.f;
		float               m_initRestitution = 0.3f;
		float               m_initFriction    = 0.5f;
		float               m_initLinearDamp  = 0.01f;
		float               m_initAngularDamp = 0.05f;
	};
}
