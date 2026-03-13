#pragma once

namespace Render
{
	class Texture
	{
	public:
		static std::unique_ptr<Texture> LoadFromFile(
			ID3D11Device*       device,
			const std::wstring& path);

		ID3D11ShaderResourceView* SRV()      const { return m_srv.Get(); }
		bool                      IsLoaded() const { return m_srv != nullptr; }

	private:
		static bool IsWicFormat(const std::wstring& path);

		RefCom<ID3D11Resource>           m_resource;
		RefCom<ID3D11ShaderResourceView> m_srv;
	};
}
