#pragma once
#include "Material.h"

namespace Render
{
	class ColorMaterial : public Material
	{
	public:
		ColorMaterial() = default;
		~ColorMaterial() = default;
	
	public:
		virtual bool Initialize(ShaderGroup* pShaderGroup) override
		{
			m_shaderGroup = pShaderGroup;
			return AutomaticRegisterPS<float4>("Color", "Color", &m_color, pShaderGroup, true);
		}

	private:
		DirectX::XMFLOAT4 m_color;
	};

}

