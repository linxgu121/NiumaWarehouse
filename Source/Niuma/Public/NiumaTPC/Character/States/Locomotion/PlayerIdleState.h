#pragma once

#include "NiumaTPC/Character/States/PlayerBaseState.h"

struct FPlayerStateContext;

class FPlayerIdleState final : public FPlayerBaseState 
{
public:
	explicit FPlayerIdleState(FPlayerStateContext& InContext);

	virtual ~FPlayerIdleState() override = default;

	virtual void Enter() override;

	virtual void LogicUpdate(float DeltaTime) override;

	virtual void PhysicsUpdate(float DeltaTiem) override;

	virtual void Exit() override;
};

