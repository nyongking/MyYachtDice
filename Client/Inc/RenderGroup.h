#pragma once
#include "ShaderGroup.h"

namespace Render
{
	// rendergroup
	// (TODO1). 필요시 MRT 변경
	class RenderGroup
	{
	public:
		RenderGroup(uint32_t id);
		~RenderGroup() = default;

	public:
		bool AddShaderGroup(std::shared_ptr<class ShaderGroup> shaderGroup);

	private:
		uint32_t m_renderID;
		std::vector<std::shared_ptr<class ShaderGroup>> m_shaderGroups;
	};

}
