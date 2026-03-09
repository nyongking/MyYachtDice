#include "GameEnginePch.h"
#include "GBufferMaterial.h"

namespace Render
{
	GBufferMaterial::GBufferMaterial(const GBufferMaterial& rhs)
		: Material(rhs)
		, m_albedoColor(rhs.m_albedoColor)
	{
	}

	bool GBufferMaterial::Initialize(ShaderGroup* shaderGroup, ID3D11Device* device)
	{
		if (!shaderGroup || !device)
			return false;

		m_shaderGroup = shaderGroup;
		m_device      = device;

		// PS: AlbedoColor CB (b0) — 텍스처 없을 때 fallback 색상
		AutomaticRegisterPS("AlbedoColor", "AlbedoColor", &m_albedoColor, shaderGroup);

		// PS: Albedo 텍스처 슬롯 등록
		AutomaticRegisterPSTexture("AlbedoMap", "gAlbedoTex", shaderGroup);

		return true;
	}

	std::unique_ptr<Material> GBufferMaterial::Clone() const
	{
		auto clone = std::make_unique<GBufferMaterial>(*this);
		clone->ChangeParameterAddress(0, &clone->m_albedoColor);
		return clone;
	}
}
