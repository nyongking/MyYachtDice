#pragma once

namespace Render
{	
	class ConstantBufferSlot
	{
	public:
		ConstantBufferSlot(class ConstantBuffer* pBuffer)
			: m_pBuffer(pBuffer)
		{

		}

	public:
		bool			ConnectVSSlot(const std::string& name, const class ShaderGroup* pShaderGroup);
		bool			ConnectPSSlot(const std::string& name, const class ShaderGroup* pShaderGroup);

		void			BindBufferToSlot();

	private:
		int				m_slotVS = -1;
		int				m_slotPS = -1;
		class ConstantBuffer* const m_pBuffer = nullptr;
	};

}

