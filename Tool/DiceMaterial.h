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

	void         SetColor(const float4& color) { m_color = color; }
	const float4& GetColor() const             { return m_color; }

private:
	float4 m_color = {};
};
