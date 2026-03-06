#include "ClientPch.h"
#include "ModelMaterial.h"

bool ModelMaterial::Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device)
{
	if (!pShaderGroup || !device)
		return false;

	m_shaderGroup = pShaderGroup;
	m_device      = device;

	// VS b0 : cbuffer "PerFrame" { float4x4 g_viewProj; }
	if (!AutomaticRegisterVS<float4x4>("ViewProj", "PerFrame", &m_viewProj, pShaderGroup))
		return false;

	// VS b1 : cbuffer "PerObject" { float4x4 g_world; }
	if (!AutomaticRegisterVS<float4x4>("World", "PerObject", &m_world, pShaderGroup))
		return false;

	return true;
}

std::unique_ptr<Render::Material> ModelMaterial::Clone() const
{
	auto clone = std::make_unique<ModelMaterial>(*this);

	if (!clone->ChangeParameterAddress<float4x4>(0, &clone->m_viewProj))
		return nullptr;

	if (!clone->ChangeParameterAddress<float4x4>(1, &clone->m_world))
		return nullptr;

	return clone;
}
