#include "RenderPch.h"
#include "ShaderGroup.h"

namespace Render
{
	ShaderGroup::ShaderGroup(std::shared_ptr<class VertexShader> vs, std::shared_ptr<class PixelShader> ps)
		: m_vertexShader(vs)
		, m_pixelShader(ps)
	{
	}

	ShaderGroup::~ShaderGroup()
	{
	}
}
