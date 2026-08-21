#include "NiumaTPC/Character/Config/Animation/PlayerLocomotionAnimationData.h"

const FPlayerMoveStartAnimation*
FPlayerDirectionalMoveStartSet::Find(
	EPlayerMoveDirection Direction) const
{
	switch (Direction)
	{
	case EPlayerMoveDirection::Forward:
		return &Forward;

	case EPlayerMoveDirection::ForwardRight:
		return &ForwardRight;

	case EPlayerMoveDirection::Right:
		return &Right;

	case EPlayerMoveDirection::BackwardRight:
		return &BackwardRight;

	case EPlayerMoveDirection::Backward:
		return &Backward;

	case EPlayerMoveDirection::BackwardLeft:
		return &BackwardLeft;

	case EPlayerMoveDirection::Left:
		return &Left;

	case EPlayerMoveDirection::ForwardLeft:
		return &ForwardLeft;

	case EPlayerMoveDirection::None:
	default:
		return nullptr;
	}
}

const FPlayerMoveStartAnimation*
UNiumaPlayerLocomotionAnimationData::FindMoveStartAnimation(
	EPlayerLocomotionState LocomotionState,
	EPlayerMoveDirection Direction) const
{
	const FPlayerDirectionalMoveStartSet* DirectionalSet = nullptr;

	switch (LocomotionState)
	{
	case EPlayerLocomotionState::Walk:
		DirectionalSet = &WalkStarts;
		break;

	case EPlayerLocomotionState::Jog:
		DirectionalSet = &JogStarts;
		break;

	case EPlayerLocomotionState::Sprint:
		DirectionalSet = &SprintStarts;
		break;

	case EPlayerLocomotionState::Idle:
	case EPlayerLocomotionState::Crouch:
	default:
		return nullptr;
	}

	return DirectionalSet->Find(Direction);
}