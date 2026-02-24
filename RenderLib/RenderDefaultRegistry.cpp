#include "RenderPch.h"
#include "RenderDefaultRegistry.h"

#include "QuadTex.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ShaderGroup.h"

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

		// QuadTex
		{
			auto geo = std::make_shared<QuadTex>();
			auto vs = std::make_shared<VertexShader>();
			auto ps = std::make_shared<PixelShader>();

			geo->DefaultCreateBuffers(pDevice);

			vs->LoadFromFile(L"./Bin/Default/Shader/VSPOSTEX.hlsl");
			vs->CreateFromShaderBlob(pDevice);

			ps->LoadFromFile(L"./Bin/Default/Shader/PSPOSTEX.hlsl");
			ps->CreateFromShaderBlob(pDevice);

			auto shaderGroup = std::make_shared<ShaderGroup>(vs, ps);

			auto item = std::make_unique<DefaultRenderItemGroup>();
			item->geometry = std::move(geo);
			item->shaderGroup = std::move(shaderGroup);

			m_defaultRenderItems.push_back(std::move(item));
		}

		return true;
	}

	std::shared_ptr<class Geometry> RenderDefaultRegistry::GetGeometry(DefaultRenderItemType type)
	{
		if (m_defaultRenderItems.size() <= type)
			return nullptr;

		return m_defaultRenderItems[type]->geometry;
	}

	std::shared_ptr<class ShaderGroup> RenderDefaultRegistry::GetShaderGroup(DefaultRenderItemType type)
	{
		if (m_defaultRenderItems.size() <= type)
			return nullptr;

		return m_defaultRenderItems[type]->shaderGroup;
	}

}

