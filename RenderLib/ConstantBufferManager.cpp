#include "RenderPch.h"
#include "ConstantBufferManager.h"
#include "RenderDevice.h"

namespace Render
{
	bool ConstantBufferManager::Initialize()
	{
		m_device = RenderDevice::GetInstance().m_device;
		m_context = RenderDevice::GetInstance().m_context;

		return true;
	}

	bool ConstantBufferManager::ReleaseBuffer(uint64_t id)
	{
		auto it = m_constantbuffers.find(id);

		if (m_constantbuffers.end() == it)
			return false;
		
		--it->second.refcount;
		if (0 == it->second.refcount)
		{
			m_constantbuffers.erase(it);
		}

		return true;
	}

	std::shared_ptr<class ConstantBuffer> ConstantBufferManager::AddOrGetConstantBuffer(const std::string& name, UINT byteWidth, const void* pInitialData, bool isDynamic)
	{
		uint64_t hashID = std::hash<std::string>{}(name);

		auto it = m_constantbuffers.find(hashID);

		// create
		if (m_constantbuffers.end() == it)
		{
			auto mem = std::make_shared<ConstantBuffer>(hashID);

			
			bool ret = false;

			if (isDynamic)
				ret = mem->Create(m_device.Get(), byteWidth, pInitialData, D3D11_USAGE_DYNAMIC);
			else
				ret = mem->Create(m_device.Get(), byteWidth, pInitialData, D3D11_USAGE_DEFAULT);

			if (!ret)
				return nullptr;

			m_constantbuffers.emplace(hashID, RefCountBlock<std::shared_ptr<class ConstantBuffer>>{1, mem});

			return mem;
		}
		else
		{
			++it->second.refcount;

			return it->second.member;
		}
	}
}