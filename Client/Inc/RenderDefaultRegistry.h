#pragma once

namespace Render
{
	enum GeometryType
	{
		GEO_QUAD_TEX = 0,
	};

	enum MaterialType
	{
		MAT_COLOR         = 0,
		MAT_DEFAULT_COLOR = 1,
	};

	enum ShaderType
	{
		SHADER_QUAD_TEX = 0,
	};

	// �ʼ������� ���� �͵��� ���⿡ �����Ѵ�.
	class RenderDefaultRegistry
	{
#pragma region Singleton
	public:
		static RenderDefaultRegistry& GetInstance()
		{
			static RenderDefaultRegistry instance;

			return instance;
		}
#pragma endregion Singleton

	public:
		RenderDefaultRegistry();
		~RenderDefaultRegistry();

	public:
		bool RegisterDefaultRenderItems(ID3D11Device* pDevice); // Initialize

		std::shared_ptr<class Geometry>		GetGeometry(GeometryType type);
		std::shared_ptr<class ShaderGroup>	GetShaderGroup(ShaderType type);
		std::shared_ptr<class Material>		GetMaterial(MaterialType type);

	private:
		std::vector<std::shared_ptr<class Material>>			m_materials;
		std::vector<std::shared_ptr<class Geometry>>			m_geometries;
		std::vector<std::shared_ptr<class ShaderGroup>>			m_shaders;
	};

}