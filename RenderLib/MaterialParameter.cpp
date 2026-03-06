#include "RenderPch.h"
#include "MaterialParameter.h"
#include "ConstantBuffer.h"

namespace Render
{
	bool ConstantBufferParameter::ChangeAddress(void* pData, uint32_t size)
	{
		if (nullptr == pData || !m_buffer || size != m_buffer->Size())
			return false;

		m_pData = pData;

		return true;
	}
	bool ConstantBufferParameter::WriteValue(const void* src, uint32_t size)
	{
		if (!src || !m_pData || size != m_dataSize)
			return false;

		memcpy(m_pData, src, size);
		return true;
	}

	void ConstantBufferParameter::Update(ID3D11DeviceContext* ctx)
	{
		if (nullptr == ctx || !m_buffer || nullptr == m_pData)
			return;

		m_buffer->Update(ctx, m_pData, m_buffer->Size());
	}
}
