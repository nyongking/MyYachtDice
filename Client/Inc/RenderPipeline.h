#pragma once

// item들
// - mesh*
// - material*
// - transform(world)

// pipeline에 필요한거
// 1. shaders(vs, gs, ps.. 등)
// 2. world buffers ? 
// - custom samplers (TODO)

// 중요한건 renderorder 순 정렬 -> shader group 별로 정렬!
// 그러면 renderItem -> material -> shader group을 참조해서 등록..

// shader group
// 1. shaders (vs, gs, ps... 등)
// - render order에 등록 ->

class RenderPipeline
{
	
};

