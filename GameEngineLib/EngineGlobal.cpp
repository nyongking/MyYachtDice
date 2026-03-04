#include "GameEnginePch.h"
#include "EngineGlobal.h"
#include "RenderDevice.h"

#include "GeometryManager.h"
#include "ShaderManager.h"

namespace GameEngine
{
	bool InitEngine()
	{
		if (!GeometryManager::GetInstance()->Initialize(
			Render::RenderDevice::GetInstance().GetDevice(),
			Render::RenderDevice::GetInstance().GetContext()))
			return false;

		if (!ShaderManager::GetInstance()->Initialize(
			Render::RenderDevice::GetInstance().GetDevice(),
			Render::RenderDevice::GetInstance().GetContext()))
			return false;



		return true;
	}
}
