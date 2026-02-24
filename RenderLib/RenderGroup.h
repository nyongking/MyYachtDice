#pragma once
#include "ShaderGroup.h"

namespace Render
{
	// rendergroup (renderorder 별 정렬)
	// 1. shader group'들'을 가진다.
	// 2. structred buffer(world transform 모음)
	// (TODO1). 필요시 MRT 변경
	class RenderGroup
	{
	public:
		

	private:
		std::vector<std::unique_ptr<class ShaderGroup>> m_shaderGroups;
	};

}
