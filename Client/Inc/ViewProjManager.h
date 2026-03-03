#pragma once
#include "ViewProjMatrix.h"

namespace Render
{
	class ViewProjManager
	{
#pragma region Singleton
	public:
		static ViewProjManager& GetInstance()
		{
			static ViewProjManager instance;

			return instance;
		}
#pragma endregion Singleton
	public:
		ViewProjManager() = default;
		~ViewProjManager() = default;

		uint32_t CreateViewProj();
		uint32_t RegisterViewProj(std::shared_ptr<class ViewProjMatrix> viewproj);
		
		bool	 ChangeCurrent(uint32_t id);		
		
		const float4x4* GetViewByID(uint32_t id) const;
		const float4x4* GetProjByID(uint32_t id) const;
		const float4x4* GetViewProjByID(uint32_t id) const;
		const float4x4* GetInverseViewByID(uint32_t id) const;

		// m_currentID는 ChangeCurrent가 저장한 0-based 인덱스
		const float4x4* GetCurrentView() const { return (m_currentID < m_viewprojs.size()) ? m_viewprojs[m_currentID]->GetView() : nullptr; }
		const float4x4* GetCurrentProj() const { return (m_currentID < m_viewprojs.size()) ? m_viewprojs[m_currentID]->GetProj() : nullptr; }
		const float4x4* GetCurrentViewProj() const { return (m_currentID < m_viewprojs.size()) ? m_viewprojs[m_currentID]->GetViewProj() : nullptr; }
		const float4x4* GetCurrentInverseView() const { return (m_currentID < m_viewprojs.size()) ? m_viewprojs[m_currentID]->GetInverseView() : nullptr; }

		bool UpdateViewByID(uint32_t id, fmatrix view);
		bool UpdateProjByID(uint32_t id, fmatrix proj);
		bool UpdateViewProjByID(uint32_t id, fmatrix view, cmatrix proj);


	private:
		uint32_t m_currentID = 0;

		std::vector<std::shared_ptr<class ViewProjMatrix>> m_viewprojs;
	};

}

