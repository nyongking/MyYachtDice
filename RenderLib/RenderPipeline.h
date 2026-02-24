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
	enum RenderOrderRegistered // RenderGroup Default
	{
		PRIORITY = 0,
		DEFAULT = 4,
		// TODO: 기본 추가 예정.
	};

	class RenderPipeline
	{
	public:
		RenderPipeline() = default;
		~RenderPipeline() = default;
	
	public:
		bool Initialize();

	private:
		std::map<uint32_t, std::unique_ptr<class RenderGroup>> m_renderGroups;
	};
}

