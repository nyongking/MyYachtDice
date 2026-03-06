#pragma once
#include "Material.h"

class ModelMaterial : public Render::Material
{
public:
	ModelMaterial()  = default;
	~ModelMaterial() = default;

public:
	bool                              Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device) override;
	std::unique_ptr<Render::Material> Clone() const override;

	void SetViewProj(const float4x4& vp) override { m_viewProj = vp; }
	void SetWorld(const float4x4& w)     override { m_world = w; }

private:
	float4x4 m_viewProj = {};
	float4x4 m_world    = {};
};
