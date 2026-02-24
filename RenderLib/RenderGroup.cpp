#include "RenderPch.h"
#include "RenderGroup.h"

namespace Render
{
	RenderGroup::RenderGroup(uint32_t id)
		: m_renderID(id)
	{
	}

	bool RenderGroup::AddShaderGroup(std::shared_ptr<class ShaderGroup> shaderGroup)
	{
		for (auto& mem : m_shaderGroups)
		{
			// 이미 등록된 shaderGroup
			if (mem == shaderGroup || *mem.get() == *shaderGroup.get())
				return false;
		}

		// 등록하려는 shaderGroup의 renderID를 바꿔야 한다.
		// renderGroupID << 16 | member value;
		uint32_t memberValue = static_cast<uint32_t>(m_shaderGroups.size());
		shaderGroup->m_renderID = (m_renderID << 16 | memberValue);

		m_shaderGroups.push_back(shaderGroup);

		return true;
	}

}

