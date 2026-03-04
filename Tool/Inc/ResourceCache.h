#pragma once
#include "JobQueue.h"

namespace GameEngine
{
    enum class ResourceState { NotLoaded, Loading, Ready, Failed };

    template<typename T>
    class ResourceCache : public JobQueue
    {
    protected:
        struct Entry
        {
            std::unique_ptr<T> resource;
            ResourceState      state = ResourceState::NotLoaded;
        };

        T* GetCached(const std::string& key) const
        {
            ReadLockGuard lock(m_cacheLock, "ResourceCache");
            auto it = m_cache.find(key);
            if (it == m_cache.end() || it->second.state != ResourceState::Ready)
                return nullptr;
            return it->second.resource.get();
        }

        mutable Lock                           m_cacheLock;
        std::unordered_map<std::string, Entry> m_cache;
    };
}
