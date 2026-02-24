#pragma once

namespace Render
{
	enum DefaultRenderItemType
	{
		QUAD_TEX = 0,
	};

	struct DefaultRenderItemGroup
	{
		std::shared_ptr<class Geometry>		geometry;
		std::shared_ptr<class ShaderGroup>	shaderGroup;
	};

	// 필수적으로 쓰일 것들을 여기에 저장한다.
	class RenderDefaultRegistry
	{
#pragma region Singleton
	public:
		static RenderDefaultRegistry& GetInstance()
		{
			static RenderDefaultRegistry instance;

			return instance;
		}
#pragma endregion Singleton

	public:
		RenderDefaultRegistry();
		~RenderDefaultRegistry();

	public:
		bool RegisterDefaultRenderItems(ID3D11Device* pDevice); // Initialize
		
		std::shared_ptr<class Geometry>		GetGeometry(DefaultRenderItemType type);
		std::shared_ptr<class ShaderGroup>	GetShaderGroup(DefaultRenderItemType type);

	private:
		std::vector<std::unique_ptr<DefaultRenderItemGroup>> m_defaultRenderItems;
	};

}