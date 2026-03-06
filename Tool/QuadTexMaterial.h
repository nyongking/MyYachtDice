#pragma once
#include "Material.h"

namespace Render { class Material; class ShaderGroup; }

class QuadTexMaterial : public Render::Material
{
public:
	QuadTexMaterial() = default;
	~QuadTexMaterial() = default;

public:
	virtual bool Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device) override;
	virtual std::unique_ptr<Render::Material> Clone() const override;

	void SetViewProj(const float4x4& vp) override { m_viewProj = vp; }
	void SetWorld(const float4x4& w)     override { m_world = w; }

private:
	float4x4	m_viewProj = {};
	float4x4	m_world = {};
};

