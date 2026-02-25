#pragma once
#include "RefCountBlock.h"
#include "ConstantBuffer.h"

namespace Render
{
	// 1. Device 및 Context 보유
	// 2. key 및 Type 제공 시 buffer 생성/반환
	// 3. 
	class ConstantBufferManager
	{
#pragma region Singleton
	public:
		static ConstantBufferManager& GetInstance()
		{
			static ConstantBufferManager instance;

			return instance;
		}
#pragma endregion Singleton
	public:
		ConstantBufferManager() = default;
		~ConstantBufferManager() = default;

	public:
		bool				Initialize();
		bool				ReleaseBuffer(uint64_t id);

		std::shared_ptr<class ConstantBuffer> AddOrGetConstantBuffer(const std::string& name, UINT byteWidth, const void* pInitialData, bool isDynamic);

	private:
		RefCom<ID3D11Device>		m_device;
		RefCom<ID3D11DeviceContext> m_context;
		std::unordered_map<uint64_t, RefCountBlock<std::shared_ptr<class ConstantBuffer>>> m_constantbuffers;
	};
}


