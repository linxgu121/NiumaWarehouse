#include "NiumaTPC/Character/RuntimeData/PlayerRuntimeData.h"

bool FPlayerRuntimeData::HasMoveInput() const
{
	return !MoveInput.IsNearlyZero();
}

void FPlayerRuntimeData::ResetFrameIntents()
{
	bWantsToJump = false;
}
