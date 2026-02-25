#pragma once

namespace Render
{
	class Renderer
	{
#pragma region Singleton
	public:
		static Renderer& GetInstance()
		{
			static Renderer instance;

			return instance;
		}
#pragma endregion Singleton

	public:
		Renderer();
		~Renderer();

	public:
		bool Initialize(bool window, UINT sizeX, UINT sizeY, HWND hwnd);
		bool CreateSamplers();

		void RenderBegin(DirectX::XMFLOAT4 clearColor = DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f));		// RTV, DSV 클리어
		void RenderEnd();		// 화면 바꾸기

	private:
		RefCom<ID3D11Device> m_device = nullptr;
		RefCom<ID3D11DeviceContext> m_context = nullptr;

		RefCom<IDXGISwapChain>			m_swapchain;
		RefCom<ID3D11RenderTargetView>	m_backbufferRTV;
		RefCom<ID3D11Texture2D>			m_depthTexture;
		RefCom<ID3D11DepthStencilView>	m_depthstencilView;

		std::vector<RefCom<ID3D11SamplerState>> m_samplers;
	};
}
