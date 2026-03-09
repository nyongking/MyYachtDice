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

private:
};
