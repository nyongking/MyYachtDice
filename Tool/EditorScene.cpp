#include "ClientPch.h"
#include "EditorScene.h"
#include "SceneManager.h"
#include "CameraComponent.h"
#include "EngineGlobal.h"

#ifdef TOOL

namespace GameEngine
{

void EditorScene::Update(float dt)
{
	Scene::Update(dt);

	BuildHierarchyPanel();
	BuildInspectorPanel();
}

void EditorScene::BuildHierarchyPanel()
{
	ImGui::Begin("Hierarchy");

#ifdef _DEBUG
	ImGui::Checkbox("Draw Colliders", &GameEngine::GDebugDrawColliders);
	ImGui::Separator();
#endif

	for (auto& go : m_gameObjects)
	{
		bool selected = (go.get() == m_selectedGO);
		if (ImGui::Selectable(go->GetName().c_str(), selected))
			m_selectedGO = go.get();
	}

	ImGui::End();
}

void EditorScene::BuildInspectorPanel()
{
	ImGui::Begin("Inspector");

	if (!m_selectedGO)
	{
		ImGui::TextDisabled("(선택된 오브젝트 없음)");
		ImGui::End();
		return;
	}

	ImGui::Text("%s", m_selectedGO->GetName().c_str());
	ImGui::Separator();

	// ── Transform ──
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::RadioButton("Translate", m_gizmoOp == ImGuizmo::TRANSLATE)) m_gizmoOp = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate",    m_gizmoOp == ImGuizmo::ROTATE))    m_gizmoOp = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale",     m_gizmoOp == ImGuizmo::SCALE))     m_gizmoOp = ImGuizmo::SCALE;

		if (m_gizmoOp != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("World", m_gizmoMode == ImGuizmo::WORLD)) m_gizmoMode = ImGuizmo::WORLD;
			ImGui::SameLine();
			if (ImGui::RadioButton("Local", m_gizmoMode == ImGuizmo::LOCAL)) m_gizmoMode = ImGuizmo::LOCAL;
		}

		ImGui::Separator();

		auto* tr  = m_selectedGO->GetTransform();
		float3 pos = tr->GetPosition();
		float3 rot = tr->GetRotation();
		float3 scl = tr->GetScale();

		if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) tr->SetPosition(pos);
		if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f))  tr->SetRotation(rot);
		if (ImGui::DragFloat3("Scale",    &scl.x, 0.01f)) tr->SetScale(scl);
	}

	// ── Components ──
	m_selectedGO->ForEachComponent([](Component* comp) {
		ImGui::PushID(comp);
		if (ImGui::CollapsingHeader(comp->GetTypeName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			comp->OnInspectorGUI();
		ImGui::PopID();
	});

	// ── 서브클래스 확장 ──
	OnInspectorFooter(m_selectedGO);

	ImGui::End();

	// ── Gizmo ──
	auto* cam = SceneManager::GetInstance().GetMainCamera();
	if (!cam || !m_selectedGO)
		return;

	const auto* pView = cam->GetView();
	const auto* pProj = cam->GetProj();
	if (!pView || !pProj)
		return;

	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList(vp));
	ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);

	auto* tr = m_selectedGO->GetTransform();
	float4x4 worldMat = tr->GetWorldMatrix();
	ImGuizmo::MODE mode = (m_gizmoOp == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : m_gizmoMode;

	if (ImGuizmo::Manipulate(
		reinterpret_cast<const float*>(pView),
		reinterpret_cast<const float*>(pProj),
		m_gizmoOp, mode,
		reinterpret_cast<float*>(&worldMat)))
	{
		float t[3], r[3], s[3];
		ImGuizmo::DecomposeMatrixToComponents(reinterpret_cast<float*>(&worldMat), t, r, s);
		tr->SetPosition({ t[0], t[1], t[2] });
		tr->SetRotation({ r[0], r[1], r[2] });
		tr->SetScale   ({ s[0], s[1], s[2] });
	}
}

} // namespace GameEngine

#endif // TOOL
