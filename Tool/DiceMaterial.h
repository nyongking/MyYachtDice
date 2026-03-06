#pragma once
#include "Material.h"

class DiceMaterial : public Render::Material
{
public:
	DiceMaterial()  = default;
	~DiceMaterial() = default;

public:
	bool                              Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device) override;
	std::unique_ptr<Render::Material> Clone() const override;

	void SetViewProj(const float4x4& vp) override { m_viewProj = vp; }
	void SetWorld(const float4x4& w)     override { m_world = w; }

	void         SetColor(const float4& color) { m_color = color; }
	const float4& GetColor() const             { return m_color; }

private:
	float4x4 m_viewProj = {};
	float4x4 m_world    = {};
	float4	 m_color = {};
};
