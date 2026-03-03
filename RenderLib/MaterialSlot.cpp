#include "RenderPch.h"
#include "MaterialSlot.h"
#include "ShaderGroup.h"
#include "ConstantBuffer.h"
#include "RenderDevice.h"

namespace Render
{
	bool ConstantBufferSlot::ConnectVSSlot(const std::string& name, const ShaderGroup* pShaderGroup)
	{
		if (nullptr == pShaderGroup)
			return false;

		m_slotVS = pShaderGroup->VSConstantBufferSlot(name, m_pBuffer->Size());

		return m_slotVS >= 0;
	}

	bool ConstantBufferSlot::ConnectPSSlot(const std::string& name, const ShaderGroup* pShaderGroup)
	{
		if (nullptr == pShaderGroup)
			return false;

		m_slotPS = pShaderGroup->PSConstantBufferSlot(name, m_pBuffer->Size());

		return m_slotPS >= 0;
	}

	void ConstantBufferSlot::BindBufferToSlot()
	{
		if (nullptr == m_pBuffer)
			return;

		auto* ctx = RenderDevice::GetInstance().GetContext();

		if (m_slotVS >= 0)
			m_pBuffer->BindVS(ctx, static_cast<uint32_t>(m_slotVS));

		if (m_slotPS >= 0)
			m_pBuffer->BindPS(ctx, static_cast<uint32_t>(m_slotPS));
	}
}

