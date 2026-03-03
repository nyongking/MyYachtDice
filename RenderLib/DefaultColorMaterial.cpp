#include "RenderPch.h"
#include "DefaultColorMaterial.h"

namespace Render
{
	bool DefaultColorMaterial::Initialize(ShaderGroup* pShaderGroup)
	{
		if (nullptr == pShaderGroup)
			return false;

		m_shaderGroup = pShaderGroup;

		// VS b0 : cbuffer "PerFrame" { float4x4 g_viewProj; }
		if (!AutomaticRegister<float4x4>("ViewProj", "PerFrame", &m_viewProj, pShaderGroup, true))
			return false;

		// VS b1 : cbuffer "PerObject" { float4x4 g_world; }
		if (!AutomaticRegister<float4x4>("World", "PerObject", &m_world, pShaderGroup, true))
			return false;

		// PS b0 : cbuffer "ColorBuffer" { float4 g_color; }
		if (!AutomaticRegisterPS<float4>("Color", "ColorBuffer", &m_color, pShaderGroup, true))
			return false;

		return true;
	}
}
