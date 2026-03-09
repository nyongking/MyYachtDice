#include "ClientPch.h"
#include "ModelMaterial.h"

bool ModelMaterial::Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device)
{
	if (!pShaderGroup || !device)
		return false;

	m_shaderGroup = pShaderGroup;
	m_device      = device;

	return true;
}

std::unique_ptr<Render::Material> ModelMaterial::Clone() const
{
	return std::make_unique<ModelMaterial>(*this);
}
