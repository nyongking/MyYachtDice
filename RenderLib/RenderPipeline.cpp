#include "RenderPch.h"
#include "RenderPipeline.h"

#include "RenderDefaultRegistry.h"

namespace Render
{
	uint32_t CalculateRenderOrder(RenderOrder order, uint32_t argument)
	{
		uint32_t renderOrder = order;

		switch (order)
		{
		case RenderOrder::PRIORITY:
		case RenderOrder::DEFAULT:
		{
			break;
		}
		case RenderOrder::NEXT_PRIORITY:
		case RenderOrder::NEXT_DEFAULT:
		{
			renderOrder += argument;
			break;
		}
		}

		return renderOrder;
	}

	bool RenderPipeline::Initialize()
	{
		RegisterRenderGroup(PRIORITY, 0);
		RegisterRenderGroup(DEFAULT, 0);

		// TODO: Priority Group

		// Default Group
		{
			if (false == AddShaderGroup(DEFAULT, 0, RenderDefaultRegistry::GetInstance().GetShaderGroup(QUAD_TEX)))
				return false;
		}

		return true;
	}

	bool RenderPipeline::RegisterRenderGroup(RenderOrder order, uint32_t argument)
	{	
		uint32_t renderOrder = CalculateRenderOrder(order, argument);

		if (m_renderGroups.find(renderOrder) != m_renderGroups.end())
			return false;

		m_renderGroups.emplace(renderOrder, std::make_unique<RenderGroup>(renderOrder));

		return true;
	}

	bool RenderPipeline::AddShaderGroup(RenderOrder order, uint32_t argument, std::shared_ptr<class ShaderGroup> shaderGroup)
	{
		uint32_t renderOrder = CalculateRenderOrder(order, argument);

		auto it = m_renderGroups.find(renderOrder);

		if (it == m_renderGroups.end())
			return false;

		return it->second->AddShaderGroup(shaderGroup);
	}
}