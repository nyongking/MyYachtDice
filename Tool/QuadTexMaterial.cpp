#include "ClientPch.h"
#include "QuadTexMaterial.h"

bool QuadTexMaterial::Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device)
{
	if (nullptr == pShaderGroup || nullptr == device)
		return false;

	m_shaderGroup = pShaderGroup;
	m_device      = device;

	if (!AutomaticRegisterPSTexture("Texture", "Texture", pShaderGroup))
		return false;

	return true;
}

std::unique_ptr<Render::Material> QuadTexMaterial::Clone() const
{
	return std::make_unique<QuadTexMaterial>(*this);
}
