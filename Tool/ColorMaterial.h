#pragma once
#include "Material.h"

namespace Render { class Material; class ShaderGroup; }

class ColorMaterial : public Render::Material
{
public:
	ColorMaterial() = default;
	~ColorMaterial() = default;

public:
	virtual bool Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device) override;
	virtual std::unique_ptr<Render::Material> Clone() const override;

	void SetColor(const float4& c) { m_color = c; }

private:
	float4 m_color = {};  // PS b0  (cbuffer "ColorBuffer")


};

