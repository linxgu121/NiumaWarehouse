#pragma once

#include "CoreMinimal.h"

/**
 * 状态基类，定义了状态的基本接口
 */
class FStateBase
{
public:
    virtual ~FStateBase();

    virtual void Enter() = 0;

    virtual void LogicUpdate(float DeltaTime) = 0;

    virtual void PhysicsUpdate(float DeltaTime) = 0;

    virtual void Exit() = 0;
};