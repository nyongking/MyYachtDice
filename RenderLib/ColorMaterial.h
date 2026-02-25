#pragma once
#include "Material.h"

namespace Render
{
	class ColorMaterial : public Material
	{
	public:
		ColorMaterial() = default;
		~ColorMaterial() = default;
	
	public:
		// Initialize 과정
		// 1. shader group 인자로 주면 shader group 있는 shader들을 얻어오고,
		// - resources -> 이름에 따라 bind될 위치를 알아오고 그에 맞게 slot 연결
		// - cbuffer -> cbuffer 생성 요청 혹은 가지고 오기 +  
		virtual bool Initialize(const ShaderGroup* shaderGroup) override
		{


			return true;
		}

	private:
		DirectX::XMFLOAT4 m_color;
	};

}

