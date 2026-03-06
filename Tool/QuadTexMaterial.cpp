#include "ClientPch.h"
#include "QuadTexMaterial.h"

bool QuadTexMaterial::Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device)
{
	if (nullptr == pShaderGroup || nullptr == device)
		return false;

	m_shaderGroup = pShaderGroup;
	m_device      = device;

	// VS b0 : cbuffer "PerFrame" { float4x4 g_viewProj; }
	if (!AutomaticRegisterVS<float4x4>("ViewProj", "PerFrame", &m_viewProj, pShaderGroup))
		return false;

	// VS b1 : cbuffer "PerObject" { float4x4 g_world; }
	if (!AutomaticRegisterVS<float4x4>("World", "PerObject", &m_world, pShaderGroup))
		return false;

	if (!AutomaticRegisterPSTexture("Texture", "Texture", pShaderGroup))
		return false;

	return true;
}

std::unique_ptr<Render::Material> QuadTexMaterial::Clone() const
{
	auto clone = std::make_unique<QuadTexMaterial>(*this);

	if (nullptr == clone)
		return nullptr;

	if (!clone->ChangeParameterAddress<float4x4>(0, &clone->m_viewProj))
		return nullptr;

	if (!clone->ChangeParameterAddress<float4x4>(1, &clone->m_world))
		return nullptr;

	return clone;
}
