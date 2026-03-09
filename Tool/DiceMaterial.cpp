#include "ClientPch.h"
#include "DiceMaterial.h"

bool DiceMaterial::Initialize(Render::ShaderGroup* pShaderGroup, ID3D11Device* device)
{
	if (!pShaderGroup || !device)
		return false;

	m_shaderGroup = pShaderGroup;
	m_device      = device;

	// PS b0 : cbuffer "ColorBuffer" { float4 g_color; }
	if (!AutomaticRegisterPS<float4>("Color", "ColorBuffer", &m_color, pShaderGroup))
		return false;

	return true;
}

std::unique_ptr<Render::Material> DiceMaterial::Clone() const
{
	auto clone = std::make_unique<DiceMaterial>(*this);

	if (!clone->ChangeParameterAddress<float4>(0, &clone->m_color))
		return nullptr;

	return clone;
}
