#include "RenderPch.h"
#include "RenderResourceManager.h"

#include "RenderDevice.h"

#include "VertexShader.h"
#include "PixelShader.h"

namespace Render
{
	RenderResourceManager::RenderResourceManager()
	{

	}

	RenderResourceManager::~RenderResourceManager()
	{
	}

	bool RenderResourceManager::Initialize()
	{
		// get device
		m_pDevice = RenderDevice::GetInstance().m_device.Get();
		m_pContext = RenderDevice::GetInstance().m_context.Get();

		// Load/Create Default Resources		
		if (!LoadVertexShaderBlobFromFile(L"./Bin/Default/Shader/VSPOSTEX.hlsl")) return false;
		if (!CompleteVertexShader(L"./Bin/Default/Shader/VSPOSTEX.hlsl")) return false;

		if (!LoadPixelShaderBlobFromFile(L"./Bin/Default/Shader/PSPOSTEX.hlsl")) return false;
		if (!CompletePixelShader(L"./Bin/Default/Shader/PSPOSTEX.hlsl")) return false;

		return true;
	}

	bool RenderResourceManager::LoadVertexShaderBlobFromFile(const std::wstring& path, const std::string& entry, const std::string& profile)
	{
		auto& container = m_vertexShaderContainer.resourceContainer;
		uint64_t hashValue = std::hash<std::wstring>{}(path);
		
		// find
		auto iter = container.find(hashValue);
		if (iter != container.end())
		{
#ifdef _DEBUG
			auto hashIter = m_vertexShaderContainer.BeforeHash.find(hashValue);
			
			bool isHashCollision = (hashIter != m_vertexShaderContainer.BeforeHash.end() && hashIter->second != path);

			if (isHashCollision)
			{
				std::wstring msg = L"Hash collision : " + path + L" with " + hashIter->second;
				OutputDebugStringW(msg.c_str());
			}
			else
			{
				std::wstring msg = L"Already Exist VertexShader : " + path;
				OutputDebugStringW(msg.c_str());
			}			
#endif
			return false;
		}

		// load
		std::shared_ptr<VertexShader> create = std::make_shared<VertexShader>();
		if (!create->LoadFromFile(path, entry, profile))
		{
#ifdef _DEBUG
			std::wstring msg = L"VertexShader LoadFromFile Failed : " + path;
			OutputDebugStringW(msg.c_str());
#endif
			return false;
		}		

		// register
		container.emplace(hashValue, create);
#ifdef _DEBUG
		m_vertexShaderContainer.BeforeHash.emplace(hashValue, path);
#endif

		return true;
	}
	bool RenderResourceManager::CompleteVertexShader(const std::wstring& path)
	{
		auto& container = m_vertexShaderContainer.resourceContainer;
		uint64_t hashValue = std::hash<std::wstring>{}(path);

		// find
		auto iter = container.find(hashValue);
		if (iter == container.end())
		{
#ifdef _DEBUG
			std::wstring msg = L"Invalid VertexShader : " + path;
			OutputDebugStringW(msg.c_str());
#endif
			return false;
		}

		if (!iter->second->CreateFromShaderBlob(m_pDevice))
		{
			std::wstring msg = L"VertexShader CreateFromShaderBlob Failed : " + path;
			OutputDebugStringW(msg.c_str());
			return false;
		}

		return true;
	}
	std::shared_ptr<class VertexShader> RenderResourceManager::GetVertexShader(const std::wstring& path)
	{
		// find
		auto iter = m_vertexShaderContainer.resourceContainer.find(std::hash<std::wstring>{}(path));

		if (iter == m_vertexShaderContainer.resourceContainer.end())
			return nullptr;

		//if (!iter->second->IsShaderCreated())
		//	return nullptr;

		return iter->second;
	}

	bool RenderResourceManager::LoadPixelShaderBlobFromFile(const std::wstring& path, const std::string& entry, const std::string& profile)
	{
		auto& container = m_pixelShaderContainer.resourceContainer;
		uint64_t hashValue = std::hash<std::wstring>{}(path);

		// find
		auto iter = container.find(hashValue);
		if (iter != container.end())
		{
#ifdef _DEBUG
			auto hashIter = m_pixelShaderContainer.BeforeHash.find(hashValue);

			bool isHashCollision = (hashIter != m_pixelShaderContainer.BeforeHash.end() && hashIter->second != path);

			if (isHashCollision)
			{
				std::wstring msg = L"Hash collision : " + path + L" with " + hashIter->second;
				OutputDebugStringW(msg.c_str());
			}
			else
			{
				std::wstring msg = L"Already Exist PixelShader : " + path;
				OutputDebugStringW(msg.c_str());
			}
#endif
			return false;
		}

		// load
		std::shared_ptr<PixelShader> create = std::make_shared<PixelShader>();
		if (!create->LoadFromFile(path, entry, profile))
		{
#ifdef _DEBUG
			std::wstring msg = L"PixelShader LoadFromFile Failed : " + path;
			OutputDebugStringW(msg.c_str());
#endif
			return false;
		}

		// register
		container.emplace(hashValue, create);
#ifdef _DEBUG
		m_pixelShaderContainer.BeforeHash.emplace(hashValue, path);
#endif

		return true;
	}
	bool RenderResourceManager::CompletePixelShader(const std::wstring& path)
	{
		auto& container = m_pixelShaderContainer.resourceContainer;
		uint64_t hashValue = std::hash<std::wstring>{}(path);

		// find
		auto iter = container.find(hashValue);
		if (iter == container.end())
		{
#ifdef _DEBUG
			std::wstring msg = L"Invalid PixelShader : " + path;
			OutputDebugStringW(msg.c_str());
#endif
			return false;
		}

		if (!iter->second->CreateFromShaderBlob(m_pDevice))
		{
			std::wstring msg = L"PixelShader CreateFromShaderBlob Failed : " + path;
			OutputDebugStringW(msg.c_str());
			return false;
		}

		return true;
	}
	std::shared_ptr<class PixelShader> RenderResourceManager::GetPixelShader(const std::wstring& path)
	{
		auto iter = m_pixelShaderContainer.resourceContainer.find(std::hash<std::wstring>{}(path));

		if (iter == m_pixelShaderContainer.resourceContainer.end())
			return nullptr;

		//if (!iter->second->IsShaderCreated())
		//	return nullptr;

		return iter->second;
	}
}


