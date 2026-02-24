#pragma once
#include "RenderGroup.h"

// item들
// - mesh*
// - material*
// - transform(world)

// 중요한건 renderorder 순 정렬 -> shader group 별로 정렬!
// 그러면 renderItem -> material -> shader group을 참조해서 등록..

namespace Render
{
	enum RenderOrder // RenderGroup Default
	{
		PRIORITY = 1,
		NEXT_PRIORITY = 2,
		DEFAULT = 4,
		NEXT_DEFAULT = 5,

		// TODO: 기본 추가 예정.
	};

	class RenderPipeline
	{
#pragma region Singleton
	public:
		static RenderPipeline& GetInstance()
		{
			static RenderPipeline instance;

			return instance;
		}
#pragma endregion Singleton

	public:
		RenderPipeline() = default;
		~RenderPipeline() = default;
	
	public:
		bool Initialize();
		bool RegisterRenderGroup(RenderOrder order, uint32_t argument);
		bool AddShaderGroup(RenderOrder order, uint32_t argument, std::shared_ptr<class ShaderGroup> shaderGroup);

	private:
		std::map<uint32_t, std::unique_ptr<class RenderGroup>> m_renderGroups;
	};
}

