#pragma once

namespace Render
{
	// shader group
	// 1. shaders (vs, gs, ps... 등)
	// 
	// 함수 1. 특정 render order에 등록(pipeline), render number 추출

	class ShaderGroup
	{
		uint32_t shaderGroupNumber = 0;
		// 추후 등록 (GS..)
		std::shared_ptr<class VertexShader> m_vertexShader;
		std::shared_ptr<class PixelShader> m_pixelShader;
	};


}
