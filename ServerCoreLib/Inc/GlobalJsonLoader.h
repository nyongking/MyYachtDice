#pragma once
#include "JobQueue.h"
#include "JsonUtil.h"

enum class JSONLOAD_STATE
{
	INVALID,
	PENDING,
    EMPTY,
	READY,
	FAILED,
};

struct JsonSlot
{
    JsonSlot() = default;
    JsonSlot(const JsonSlot& rhs)
        : state(rhs.state.load())
        , path(rhs.path)
        , type(rhs.type)
        , result(rhs.result)
        , error(rhs.error)
    {}

    Atomic<JSONLOAD_STATE> state{ JSONLOAD_STATE::INVALID };
    exwstring path;

    TypeIndex type{ typeid(void) };
    Ref<void> result;
    exwstring error;
};

class CORE_DLL GlobalJsonLoader : public JobQueue
{
public:    
    template<class T>
    uint32 RequestLoad(bool deferred, const exwstring& path);
    template<class T>
    Ref<T>  TryGet(uint32 handle);


    bool    IsLoaded(uint32 handle) const;
    bool    CompleteLoad(uint32 handle);
private:
    uint32  AllocSlot(const exwstring& path);

private:
    USE_LOCKS(2);
    exvector<JsonSlot>          m_slots;
    exstack<uint32>             m_indices;
};

template<class T>
inline Ref<T> GlobalJsonLoader::TryGet(uint32 handle)
{
    {
        READ_LOCK_IDX(0);

        if (handle >= m_slots.size())
        {
            throw std::runtime_error("Invalid Handle");
        }
    }

    const JsonSlot& slot = m_slots[handle];

    if (slot.state.load() != JSONLOAD_STATE::READY)
        return {};
    if (slot.type != TypeIndex(typeid(T)))
        return {};
    return std::static_pointer_cast<T>(slot.result);
}

template<class T>
uint32 GlobalJsonLoader::RequestLoad(bool deferred, const exwstring& path)
{
    uint32 ret = 0;
    m_locks[1].WriteLock(typeid(this).name());

    if (m_indices.empty())
    {
        m_locks[1].WriteUnlock(typeid(this).name());
        ret = AllocSlot(path);
    }
    else
    {
        ret = m_indices.top();
        m_indices.pop();

        m_locks[1].WriteUnlock(typeid(this).name());
        
        m_slots[ret].state = JSONLOAD_STATE::PENDING;
        m_slots[ret].path = path;
    }

    m_slots[ret].type = std::type_index(typeid(T));

    DoAsync(deferred, [this, ret, path]()
        {
            JsonSlot& slot = m_slots[ret];

            try
            {
                if (slot.state != JSONLOAD_STATE::PENDING)
                {
                    throw std::runtime_error("Handle is not pending");
                }

                MyJson retJson;
                if (false == LoadJson(path.c_str(), retJson))
                {
                    throw std::runtime_error("Cannot open the file");
                }

                T dest = retJson.get<T>(); // from_json

                slot.result = MakeShared<T>(std::move(dest));
                slot.state = JSONLOAD_STATE::READY;
            }
            catch (const std::exception& e)
            {
                slot.error = exwstring(L" path : ") + path;
                slot.state = JSONLOAD_STATE::FAILED;
            }

        });

    return ret;
}

