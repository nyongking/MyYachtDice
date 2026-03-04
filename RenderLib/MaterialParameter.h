#pragma once

namespace Render
{
	class ConstantBufferParameter
	{
	public:
		ConstantBufferParameter(void* pData, std::shared_ptr<class ConstantBuffer>&& buffer)
			: m_pData(pData)
			, m_buffer(std::move(buffer))
		{

		}

		ConstantBufferParameter(const ConstantBufferParameter& rhs)
			: m_buffer(rhs.m_buffer)
		{

		}

	public:
		ConstantBuffer* Buffer() { return m_buffer.get(); }

		void Update(ID3D11DeviceContext* ctx);
		bool Release();


	private:
		void* 												m_pData = nullptr;
		std::shared_ptr<class ConstantBuffer>				m_buffer;
	};
}
