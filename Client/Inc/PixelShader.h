#pragma once
#include "Shader.h"

namespace Render
{
	class PixelShader : public Shader
	{
	public:
		PixelShader() = default;
		~PixelShader() = default;

		// Shader을(를) 통해 상속됨
		bool LoadFromFile(const std::wstring& path, const std::string& entry = "main", const std::string& profile = "ps_5_0") override;
		bool CreateFromShaderBlob(ID3D11Device* pDevice) override;

	private:
		RefCom<ID3D11PixelShader> m_pixelShader;
	};
}


