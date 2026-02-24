#pragma once

#include "d3dcompiler.h"
#pragma comment(lib, "d3dcompiler.lib")

namespace Render
{
	struct ResourceBindInfo
	{
		std::string name;
		D3D_SHADER_INPUT_TYPE type;
		uint32_t slot;
	};

	struct ShaderVariableInfo
	{
		std::string name;
		uint32_t startOffset;
		uint32_t size;
	};

	struct ConstantBufferInfo
	{
		std::string name;
		uint32_t slot;
		uint32_t size;
		std::vector<ShaderVariableInfo> variables;
	};


	class Shader abstract
	{
	public:
		Shader() = default;
		virtual ~Shader() = default;

		bool IsShaderCreated() { return m_isCreated; }
		bool ReflectShader();

		virtual bool LoadFromFile(const std::wstring& path, const std::string& entry = "main", const std::string& profile = "") abstract;
		virtual bool CreateFromShaderBlob(ID3D11Device* pDevice) abstract;
		
	protected:
		bool			 m_isCreated = false;

		std::vector<ConstantBufferInfo> m_cbuffers;
		std::vector<ResourceBindInfo> m_resources;

		RefCom<ID3DBlob> m_shaderBlob;
		RefCom<ID3DBlob> m_errorBlob;
		RefCom<ID3D11ShaderReflection> m_reflection;


	};

}

