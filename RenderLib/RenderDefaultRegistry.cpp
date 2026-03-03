#include "RenderPch.h"
#include "RenderDefaultRegistry.h"

#include "QuadTex.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ShaderGroup.h"

#include "ColorMaterial.h"
#include "DefaultColorMaterial.h"

namespace Render
{
	RenderDefaultRegistry::RenderDefaultRegistry()
	{
	}

	RenderDefaultRegistry::~RenderDefaultRegistry()
	{
	}

	bool RenderDefaultRegistry::RegisterDefaultRenderItems(ID3D11Device* pDevice)
	{
		if (nullptr == pDevice)
			return false;

		// geometry
		{
			auto geo = std::make_shared<QuadTex>();

			if (false == geo->DefaultCreateBuffers(pDevice))
				return false;

			m_geometries.push_back(std::move(geo));
		}

		// shader
		{
			auto vs = std::make_shared<VertexShader>();
			auto ps = std::make_shared<PixelShader>();

			if (false == vs->LoadFromFile(L"./Bin/Default/Shader/VSPOSTEX.hlsl"))
				return false;
			if (false == vs->CreateFromShaderBlob(pDevice))
				return false;

			if (false == ps->LoadFromFile(L"./Bin/Default/Shader/PSPOSTEX.hlsl"))
				return false;
			if (false == ps->CreateFromShaderBlob(pDevice))
				return false;

			auto shaderGroup = std::make_shared<ShaderGroup>();
			if (false == shaderGroup->Initialize(vs, ps, pDevice))
				return false;

			m_shaders.push_back(std::move(shaderGroup));
		}

		// material
		{
			// MAT_COLOR (index 0)
			auto colorMat = std::make_shared<ColorMaterial>();

			if (false == colorMat->Initialize(m_shaders[SHADER_QUAD_TEX].get()))
				return false;

			m_materials.push_back(std::move(colorMat));

			// MAT_DEFAULT_COLOR (index 1)
			auto defaultColorMat = std::make_shared<DefaultColorMaterial>();

			if (false == defaultColorMat->Initialize(m_shaders[SHADER_QUAD_TEX].get()))
				return false;

			m_materials.push_back(std::move(defaultColorMat));
		}

		return true;
	}

	std::shared_ptr<class Geometry> RenderDefaultRegistry::GetGeometry(GeometryType type)
	{
		if (static_cast<size_t>(type) >= m_geometries.size())
			return nullptr;

		return m_geometries[type];
	}

	std::shared_ptr<class ShaderGroup> RenderDefaultRegistry::GetShaderGroup(ShaderType type)
	{
		if (static_cast<size_t>(type) >= m_shaders.size())
			return nullptr;

		return m_shaders[type];
	}

	std::shared_ptr<class Material> RenderDefaultRegistry::GetMaterial(MaterialType type)
	{
		if (static_cast<size_t>(type) >= m_materials.size())
			return nullptr;

		return m_materials[type];
	}
}
