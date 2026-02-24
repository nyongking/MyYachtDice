#pragma once

namespace Render
{
	// 함수 1. 특정 render order에 등록(pipeline), render number 추출
	class ShaderGroup
	{
	public:
		ShaderGroup() = default;
		ShaderGroup(std::shared_ptr<class VertexShader> vs, std::shared_ptr<class PixelShader> ps);
		~ShaderGroup();

		friend class RenderGroup;

	public:
		bool operator==(const ShaderGroup& other)
		{
			if ((m_renderID != 0 && other.m_renderID != 0)
				&& m_renderID == other.m_renderID)
				return true;

			if (other.m_vertexShader != m_vertexShader
				|| other.m_pixelShader != m_pixelShader)
				return false;

			return true;
		}

	private:
		uint32_t		m_renderID = 0;
		// 추후 등록 (GS..)
		std::shared_ptr<class VertexShader> m_vertexShader;
		std::shared_ptr<class PixelShader> m_pixelShader;
	};


}
