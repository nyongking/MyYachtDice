#pragma once

namespace Render { class Material; }

namespace GameEngine
{
	class GeometryManager;
	class ShaderManager;
	class MaterialManager;
	class TextureManager;
	class ModelManager;
	class PhysicsManager;

	extern GeometryManager* GGeometryManager;
	extern ShaderManager*   GShaderManager;
	extern MaterialManager* GMaterialManager;
	extern TextureManager*  GTextureManager;
	extern ModelManager*    GModelManager;
	extern PhysicsManager*  GPhysicsManager;

#ifdef _DEBUG
	extern bool GDebugDrawColliders;
	extern Render::Material* GDebugWireframeMaterial;
#endif

	bool InitEngine();
	void ReleaseEngine();
}
