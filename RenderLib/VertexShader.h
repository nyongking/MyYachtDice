#pragma once
#include "Shader.h"


namespace Render
{
	class VertexShader : public Shader
	{
	public:
		VertexShader() = default;
		~VertexShader() = default;

		bool LoadFromFile( const std::wstring& path, const std::string& entry = "main", const std::string& profile = "vs_5_0") override;
		bool CreateFromShaderBlob(ID3D11Device* pDevice) override;

	private:
		RefCom<ID3D11VertexShader> m_vertexShader;
		RefCom<ID3D11InputLayout> m_InputLayout;

	};

}


