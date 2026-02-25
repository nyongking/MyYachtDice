#pragma once
#include "ShaderGroup.h"

namespace Render
{
	// rendergroup
	// viewproj 변경 필요할 경우 변경한다.
	// (TODO1). 필요시 MRT 변경

	class RenderGroup
	{
	public:
		RenderGroup(uint32_t id);
		~RenderGroup() = default;

	public:
		bool AddShaderGroup(std::shared_ptr<class ShaderGroup> shaderGroup);

	private:
		uint32_t m_renderID = 0;

		std::vector<std::shared_ptr<class ShaderGroup>> m_shaderGroups;
	};

}
