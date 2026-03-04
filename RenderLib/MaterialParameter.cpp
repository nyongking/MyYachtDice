#include "RenderPch.h"
#include "MaterialParameter.h"
#include "ConstantBuffer.h"
#include "ConstantBufferManager.h"

namespace Render
{
	void ConstantBufferParameter::Update(ID3D11DeviceContext* ctx)
	{
		if (nullptr == ctx || nullptr == m_buffer || nullptr == m_pData)
			return;

		m_buffer->Update(ctx, m_pData, m_buffer->Size());
	}

	bool ConstantBufferParameter::Release()
	{
		if (nullptr != m_buffer)
		{
			m_buffer->Release(); // warning;
		}

		m_buffer.reset();

		return true;
	}
}
