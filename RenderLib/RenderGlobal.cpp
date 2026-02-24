#include "RenderPch.h"
#include "RenderGlobal.h"

#include "RenderDevice.h"
#include "Renderer.h"
#include "RenderResourceManager.h"
#include "RenderPipeline.h"

namespace Render
{
	bool InitRender(bool window, UINT sizeX, UINT sizeY, HWND hwnd)
	{
		if (false == RenderDevice::GetInstance().Initialize())
			return false;

		if (false == Renderer::GetInstance().Initialize(window, sizeX, sizeY, hwnd))
			return false;

		if (false == RenderResourceManager::GetInstance().Initialize())
			return false;

		if (false == RenderPipeline::GetInstance().Initialize())
			return false;

		return true;
	}

}
