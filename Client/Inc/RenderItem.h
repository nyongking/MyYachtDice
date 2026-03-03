#pragma once

namespace Render
{
	// 게임 오브젝트가 매 프레임 RenderPipeline::Submit()으로 제출하는 경량 구조체
	struct RenderCommand
	{
		class Geometry* geometry        = nullptr;
		class Material* material        = nullptr;
		const void*     cbPerObject     = nullptr;    // 월드 행렬 등 오브젝트별 데이터
		uint32_t        cbPerObjectSize = 0;
	};
}
