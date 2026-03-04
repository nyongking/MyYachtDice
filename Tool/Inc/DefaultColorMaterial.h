#pragma once
#include "Material.h"

namespace Render
{
	class DefaultColorMaterial : public Material
	{
	public:
		DefaultColorMaterial() = default;
		~DefaultColorMaterial() = default;

	public:
		virtual bool Initialize(ShaderGroup* pShaderGroup) override;

		void SetViewProj(const float4x4& vp) override { m_viewProj = vp; }
		void SetWorld(const float4x4& w)     override { m_world    = w;  }
		void SetColor(const float4& c)                { m_color    = c;  }

	private:
		float4x4 m_viewProj = {};  // VS b0  (cbuffer "PerFrame")
		float4x4 m_world    = {};  // VS b1  (cbuffer "PerObject")
		float4   m_color    = {};  // PS b0  (cbuffer "ColorBuffer")
	};
}
