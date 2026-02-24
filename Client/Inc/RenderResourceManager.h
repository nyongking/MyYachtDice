#pragma once
#include <mutex>

namespace Render
{
	template<typename T, typename StringType>
	struct ResourceContainer
	{
		std::unordered_map<uint64_t, std::shared_ptr<T>> resourceContainer;

#ifdef _DEBUG
		std::unordered_map<uint64_t, StringType> BeforeHash;
#endif
	};

	class RenderResourceManager
	{
#pragma region Singleton
	public:
		static RenderResourceManager& GetInstance()
		{
			static RenderResourceManager instance;

			return instance;
		}
#pragma endregion Singleton

	public:
		RenderResourceManager();
		~RenderResourceManager();

		bool Initialize();

		bool LoadVertexShaderBlobFromFile(const std::wstring& path, const std::string& entry = "main", const std::string& profile = "vs_5_0");
		bool CompleteVertexShader(const std::wstring& path);
		std::shared_ptr<class VertexShader> GetVertexShader(const std::wstring& path);

		bool LoadPixelShaderBlobFromFile(const std::wstring& path, const std::string& entry = "main", const std::string& profile = "ps_5_0");
		bool CompletePixelShader(const std::wstring& path);
		std::shared_ptr<class PixelShader> GetPixelShader(const std::wstring& path);

	private:
		ID3D11Device*			m_pDevice = nullptr;
		ID3D11DeviceContext*	m_pContext = nullptr;
		
		ResourceContainer<class Geometry, std::wstring>		m_geometries;
		ResourceContainer<class VertexShader, std::wstring> m_vertexShaderContainer;
		ResourceContainer<class PixelShader, std::wstring>  m_pixelShaderContainer;
	};
}
