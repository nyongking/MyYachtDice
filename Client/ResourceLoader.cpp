#include "ClientPch.h"
#include "ResourceLoader.h"
#include "RenderResourceManager.h"

ResourceLoader::ResourceLoader(uint32 asyncCount)
{
	SetPopCount(asyncCount);
}

// TODO;
bool ResourceLoader::LoadVertexShader(bool deferred, const std::wstring& path, const std::string& entry, const std::string& profile)
{
	if (deferred)
	{
		DoAsync(true, [path, entry, profile]() 
			{
				bool blobLoaded = Render::RenderResourceManager::GetInstance().LoadVertexShaderBlobFromFile(path, entry, profile);
				
				if (!blobLoaded)
					return;
			});
	}
	else
	{

	}

	return true;
}
