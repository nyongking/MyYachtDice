#pragma once
#include "JobQueue.h"
#include "JsonUtil.h"
#include "LoadDesc.h"

class CORE_DLL SingleJsonLoader : public JobQueue
{
public:
    template<typename T>
    bool RequestLoad(bool async, const exwstring& path, bool deferred = true);

    template<class T>
    Ref<T>  TryGet();

    bool    IsLoaded() const;

private:
    template<typename T>
    void Load();

private:
    LoadSlot                    m_slot;
};

template<typename T>
bool SingleJsonLoader::RequestLoad(bool async, const exwstring& path, bool deferred)
{
    if (LOAD_STATE::INVALID != m_slot.state.load())
        return false;

    m_slot.state.store(LOAD_STATE::PENDING);
    m_slot.path = path;
    m_slot.type = std::type_index(typeid(T));

    if (async)
    {
        DoAsync(deferred, [this]() { Load<T>(); });
    }
    else
    {
        Load<T>();
    }

    return true;
}

template<class T>
inline Ref<T> SingleJsonLoader::TryGet()
{
    if (m_slot.state.load() != LOAD_STATE::READY)
        return {};
    if (m_slot.type != TypeIndex(typeid(T)))
        return {};
    return std::static_pointer_cast<T>(m_slot.result);
}

template<typename T>
void SingleJsonLoader::Load()
{
    try
    {
        if (m_slot.state != LOAD_STATE::PENDING)
        {
            throw std::runtime_error("Not pending State");
        }

        MyJson retJson;
        if (false == LoadJson(m_slot.path.c_str(), retJson))
        {
            throw std::runtime_error("Cannot open the file");
        }

        T dest = retJson.get<T>(); // from_json Á¤ÀÇ

        m_slot.result = MakeShared<T>(std::move(dest));
        m_slot.state = LOAD_STATE::READY;
    }
    catch (const std::exception& e)
    {
        m_slot.error = exwstring(L" path : ") + m_slot.path;
        m_slot.state = LOAD_STATE::FAILED;
    }
}