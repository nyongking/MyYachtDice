#pragma once

namespace Render
{
	class ConstantBufferParameter
	{
		ConstantBufferParameter();

	public:


	private:
		void* const											m_pData = nullptr;
		std::shared_ptr<class ConstantBuffer>				m_pBuffer;
	};
}
