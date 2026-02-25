#pragma once
#include "RefCountBlock.h"

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
		bool Initialize();

	private:
		std::unordered_map<uint32_t, RefCountBlock<std::shared_ptr<class ConstantBuffer>>> m_constantbuffers;
	};
}


