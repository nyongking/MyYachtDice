#include "RenderPch.h"
#include "ViewProjManager.h"

namespace Render
{
	static uint32_t sID = 1;

	uint32_t ViewProjManager::CreateViewProj()
	{
		m_viewprojs.push_back(std::make_shared<ViewProjMatrix>());

		return sID++;
	}

	uint32_t ViewProjManager::RegisterViewProj(std::shared_ptr<class ViewProjMatrix> viewproj)
	{
		m_viewprojs.push_back(viewproj);

		return sID++;
	}

	bool ViewProjManager::ChangeCurrent(uint32_t id)
	{
		if (--id >= m_viewprojs.size())
			return false;

		m_currentID = id;
		return true;
	}

	const float4x4* ViewProjManager::GetViewByID(uint32_t id) const
	{
		if (--id >= m_viewprojs.size())
			return nullptr;

		return m_viewprojs[id]->GetView();
	}

	const float4x4* ViewProjManager::GetProjByID(uint32_t id) const
	{
		if (--id >= m_viewprojs.size())
			return nullptr;

		return m_viewprojs[id]->GetProj();
	}

	const float4x4* ViewProjManager::GetViewProjByID(uint32_t id) const
	{
		if (--id >= m_viewprojs.size())
			return nullptr;

		return m_viewprojs[id]->GetViewProj();
	}

	const float4x4* ViewProjManager::GetInverseViewByID(uint32_t id) const
	{
		if (--id >= m_viewprojs.size())
			return nullptr;

		return m_viewprojs[id]->GetInverseView();
	}

	bool ViewProjManager::UpdateViewByID(uint32_t id, fmatrix view)
	{
		if (--id >= m_viewprojs.size())
			return false;
		
		m_viewprojs[id]->UpdateView(view);

		return true;
	}

	bool ViewProjManager::UpdateProjByID(uint32_t id, fmatrix proj)
	{
		if (--id >= m_viewprojs.size())
			return false;

		m_viewprojs[id]->UpdateView(proj);

		return true;
	}

	bool ViewProjManager::UpdateViewProjByID(uint32_t id, fmatrix view, cmatrix proj)
	{
		if (--id >= m_viewprojs.size())
			return false;

		m_viewprojs[id]->UpdateViewProj(view, proj);

		return true;
	}

}

