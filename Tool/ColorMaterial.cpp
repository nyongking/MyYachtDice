#include "ClientPch.h"
#include "ColorMaterial.h"

std::unique_ptr<Render::Material> ColorMaterial::Clone() const
{
	auto clone = std::make_unique<ColorMaterial>(*this);

	if (nullptr == clone)
		return nullptr;

	if (!clone->ChangeParameterAddress<float4x4>(0, &clone->m_viewProj))
		return nullptr;

	if (!clone->ChangeParameterAddress<float4x4>(1, &clone->m_world))
		return nullptr;

	if (!clone->ChangeParameterAddress<float4>(2, &clone->m_color))
		return nullptr;

	return clone;
}

bool ColorMaterial::Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device)
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

	// PS b0 : cbuffer "ColorBuffer" { float4 g_color; }
	if (!AutomaticRegisterPS<float4>("Color", "ColorBuffer", &m_color, pShaderGroup))
		return false;

	return true;
}
