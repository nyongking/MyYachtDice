#pragma once
enum class LOAD_STATE
{
    INVALID,
    PENDING,
    EMPTY,
    READY,
    FAILED,
};

struct CORE_DLL LoadSlot
{
    LoadSlot() = default;
    LoadSlot(const LoadSlot& rhs)
        : state(rhs.state.load())
        , path(rhs.path)
        , type(rhs.type)
        , result(rhs.result)
        , error(rhs.error)
    {
    }

    Atomic<LOAD_STATE> state{ LOAD_STATE::INVALID };
    exstring path;

    TypeIndex type{ typeid(void) };
    Ref<void> result;
    exstring error;
};
