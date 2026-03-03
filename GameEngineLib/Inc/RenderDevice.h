#pragma once

namespace Render
{
	class RenderDevice
	{
#pragma region Singleton
	public:
		static RenderDevice& GetInstance()
		{
			static RenderDevice instance;

			return instance;
		}
#pragma endregion Singleton

		friend class Renderer;
		friend class RenderResourceManager;
		friend class ConstantBufferManager;

	public:
		RenderDevice();
		~RenderDevice();

	public:
		bool Initialize();

		ID3D11Device*        GetDevice()  { return m_device.Get(); }
		ID3D11DeviceContext* GetContext() { return m_context.Get(); }

	private:
		RefCom<ID3D11Device>		m_device;
		RefCom<ID3D11DeviceContext>	m_context;
	};


}

