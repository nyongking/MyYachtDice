#pragma once
#include "Material.h"

namespace Render
{
	// G-Buffer Pass용 Material
	// VTXPOSNORMTANUV 버텍스 + Albedo 텍스처 → RT0(Albedo), RT1(Normal), RT2(WorldPos)
	class GBufferMaterial : public Material
	{
	public:
		GBufferMaterial()  = default;
		~GBufferMaterial() = default;

		GBufferMaterial(const GBufferMaterial& rhs);

	public:
		bool Initialize(ShaderGroup* shaderGroup, ID3D11Device* device) override;
		std::unique_ptr<Material> Clone() const override;

		void SetAlbedoColor(const float4& color)         { m_albedoColor.color = color; }
		float4 GetAlbedoColor() const                    { return m_albedoColor.color; }
		bool SetAlbedoTexture(Texture* tex)              { return SetTexture("AlbedoMap", tex); }
		bool SetAlbedoTexture(ID3D11ShaderResourceView* srv) { return SetTexture("AlbedoMap", srv); }

	private:
		struct AlbedoColorData { float4 color = { 1.f, 1.f, 1.f, 1.f }; };

		AlbedoColorData m_albedoColor = {};
	};
}
