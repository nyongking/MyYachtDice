#pragma once

namespace Render
{
	enum class DefaultRenderItemType
	{
		QUAD_TEX = 0,
	};

	struct RenderItemGroup
	{

	};

	// 필수적으로 쓰일 것들을 여기에 저장한다.
	class RenderItemContainer
	{
#pragma region Singleton
	public:
		static RenderItemContainer& GetInstance()
		{
			static RenderItemContainer instance;

			return instance;
		}
#pragma endregion Singleton

	public:
		RenderItemContainer();
		~RenderItemContainer();

	public:
		bool RegisterDefaultRenderItems();

	private:

	};

}