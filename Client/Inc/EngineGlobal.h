#pragma once

namespace GameEngine
{
	class GeometryManager;
	class ShaderManager;
	class MaterialManager;
	class TextureManager;
	class ModelManager;

	extern GeometryManager* GGeometryManager;
	extern ShaderManager*   GShaderManager;
	extern MaterialManager* GMaterialManager;
	extern TextureManager*  GTextureManager;
	extern ModelManager*    GModelManager;

	bool InitEngine();
	void ReleaseEngine();
}
