#pragma once
#include "ResourceCache.h"
#include "Material.h"

namespace GameEngine
{
	class MaterialManager : public ResourceCache<Render::Material>
	{
#pragma region Singleton
    public:
        static std::shared_ptr<MaterialManager>& GetInstance()
        {
            static std::shared_ptr<MaterialManager> instance(new MaterialManager());
            return instance;
        }
    private:
        MaterialManager() = default;
        MaterialManager(const MaterialManager&) = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;
#pragma endregion Singleton

    public:
        bool Initialize();

        // 동기: shaderKey의 ShaderGroup이 Loading 상태면 Ready/Failed까지 블로킹
        Render::Material* LoadSync(const std::string& key,
                                   std::unique_ptr<Render::Material> mat,
                                   const std::string& shaderKey);

        // 비동기: 워커 스레드에서 shader 대기 후 Initialize()
        void LoadAsync(const std::string& key,
                       std::unique_ptr<Render::Material> mat,
                       const std::string& shaderKey,
                       std::function<void(Render::Material*)> onComplete = nullptr);

        // 캐시 내용의 Clone을 반환 (호출자가 독립적 소유)
        std::unique_ptr<Render::Material> Get(const std::string& key) const;
	};
}
