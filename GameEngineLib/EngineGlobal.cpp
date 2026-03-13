#include "GameEnginePch.h"
#include "EngineGlobal.h"
#include "RenderDevice.h"

#include "GeometryManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ComponentRegistry.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "ModelComponent.h"
#include "QuadComponent.h"
#include "PhysicsManager.h"
#include "RigidBodyComponent.h"

#include <objbase.h>

namespace GameEngine
{
	// 내부 소유
	static std::shared_ptr<GeometryManager> s_geometryMgr;
	static std::shared_ptr<ShaderManager>   s_shaderMgr;
	static std::shared_ptr<MaterialManager> s_materialMgr;
	static std::shared_ptr<TextureManager>  s_textureMgr;
	static std::shared_ptr<ModelManager>    s_modelMgr;
	static std::shared_ptr<PhysicsManager>  s_physicsMgr;

	// 외부 접근용 raw pointer
	GeometryManager* GGeometryManager = nullptr;
	ShaderManager*   GShaderManager   = nullptr;
	MaterialManager* GMaterialManager = nullptr;
	TextureManager*  GTextureManager  = nullptr;
	ModelManager*    GModelManager    = nullptr;
	PhysicsManager*  GPhysicsManager  = nullptr;

#ifdef _DEBUG
	bool GDebugDrawColliders = true;
	Render::Material* GDebugWireframeMaterial = nullptr;
#endif

	bool InitEngine()
	{
		// WICTextureLoader는 COM 초기화 필요
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		s_geometryMgr = std::shared_ptr<GeometryManager>(new GeometryManager());
		s_shaderMgr   = std::shared_ptr<ShaderManager>(new ShaderManager());
		s_materialMgr = std::shared_ptr<MaterialManager>(new MaterialManager());
		s_textureMgr  = std::shared_ptr<TextureManager>(new TextureManager());
		s_modelMgr    = std::shared_ptr<ModelManager>(new ModelManager());

		GGeometryManager = s_geometryMgr.get();
		GShaderManager   = s_shaderMgr.get();
		GMaterialManager = s_materialMgr.get();
		GTextureManager  = s_textureMgr.get();
		GModelManager    = s_modelMgr.get();

		auto device  = Render::RenderDevice::GetInstance().GetDevice();
		auto context = Render::RenderDevice::GetInstance().GetContext();

		if (!GGeometryManager->Initialize(device, context))
			return false;

		if (!GShaderManager->Initialize(device, context))
			return false;

		if (!GMaterialManager->Initialize(device))
			return false;

		if (!GTextureManager->Initialize(device, context))
			return false;

		if (!GModelManager->Initialize(device))
			return false;

		// Physics
		s_physicsMgr = std::shared_ptr<PhysicsManager>(new PhysicsManager());
		GPhysicsManager = s_physicsMgr.get();
		if (!GPhysicsManager->Initialize())
			return false;

		// 내장 컴포넌트 등록 — 씬 역직렬화 시 타입명으로 생성 가능
		ComponentRegistry::Register<CameraComponent>   ("CameraComponent");
		ComponentRegistry::Register<LightComponent>    ("LightComponent");
		ComponentRegistry::Register<ModelComponent>    ("ModelComponent");
		ComponentRegistry::Register<QuadComponent>     ("QuadComponent");
		ComponentRegistry::Register<RigidBodyComponent>("RigidBodyComponent");

		return true;
	}

	void ReleaseEngine()
	{
		s_physicsMgr.reset();
		s_modelMgr.reset();
		s_textureMgr.reset();
		s_materialMgr.reset();
		s_shaderMgr.reset();
		s_geometryMgr.reset();

		GPhysicsManager  = nullptr;
		GModelManager    = nullptr;
		GTextureManager  = nullptr;
		GMaterialManager = nullptr;
		GShaderManager   = nullptr;
		GGeometryManager = nullptr;

		CoUninitialize();
	}
}
