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

	public:
		RenderDevice();
		~RenderDevice();

	public:
		bool Initialize();

	private:
		RefCom<ID3D11Device>		m_device;
		RefCom<ID3D11DeviceContext>	m_context;
	};


}

