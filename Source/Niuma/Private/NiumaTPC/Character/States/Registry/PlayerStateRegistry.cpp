#include "NiumaTPC/Character/States/Registry/PlayerStateRegistry.h"

#include "NiumaTPC/Character/States/PlayerBaseState.h"

FPlayerStateRegistry::FPlayerStateRegistry() = default;

FPlayerStateRegistry::~FPlayerStateRegistry() = default;

bool FPlayerStateRegistry::RegisterState(TUniquePtr<FPlayerBaseState> State)
{
    if (!State)
    {
        UE_LOG(LogTemp,Warning,TEXT( "[PlayerStateRegistry] ""²»ÄÜ×¢²á¿Õ×´Ì¬"));

        return false;
    }

    const EPlayerStateType StateType = State->GetStateType();

    if (StateType == EPlayerStateType::None)
    {
        UE_LOG(LogTemp,Warning,TEXT("[PlayerStateRegistry] ""²»ÄÜ×¢²áNone×´Ì¬"));

        return false;
    }

    if (States.Contains(StateType))
    {
        UE_LOG(LogTemp,Warning,TEXT("[PlayerStateRegistry] ""×´Ì¬Éí·ÝÖØ¸´£º%d"),
            static_cast<uint8>(StateType));

        return false;
    }

    States.Add(StateType,MoveTemp(State));

    return true;
}

FPlayerBaseState* FPlayerStateRegistry::FindState(EPlayerStateType StateType)
{
    TUniquePtr<FPlayerBaseState>* FoundState = States.Find(StateType);

    if (!FoundState)
    {
        return nullptr;
    }

    return FoundState->Get();
}

const FPlayerBaseState* FPlayerStateRegistry::FindState(EPlayerStateType StateType) const
{
    const TUniquePtr<FPlayerBaseState>* FoundState = States.Find(StateType);

    if (!FoundState)
    {
        return nullptr;
    }

    return FoundState->Get();
}


bool FPlayerStateRegistry::ContainsState(EPlayerStateType StateType) const
{
    return States.Contains(StateType);
}

int32 FPlayerStateRegistry::Num() const
{
    return States.Num();
}

void FPlayerStateRegistry::Reset()
{
    States.Reset();
}
