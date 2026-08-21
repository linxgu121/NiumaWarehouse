#include "NiumaTPC/Character/Processing/Locomotion/PlayerMovementParameterProcessor.h"

#include "Math/RotationMatrix.h"

#include "NiumaTPC/Character/Motion/PlayerMoveDirection.h"
#include "NiumaTPC/Character/RuntimeData/PlayerRuntimeData.h"

void FPlayerMovementParameterProcessor::Update(
	FPlayerRuntimeData& RuntimeData,
	float AuthorityYawDegrees,
	const FVector& CharacterForward) const
{
	if(!RuntimeData.HasMoveInput())
	{
		RuntimeData.DesiredWorldDirection = FVector::ZeroVector;

		RuntimeData.DesiredLocalMoveAngle = 0.0f;

		RuntimeData.DesiredMoveDirection = EPlayerMoveDirection::None;

		return;
	}

	RuntimeData.DesiredWorldDirection = CalculateWorldDirection(
		RuntimeData.MoveInput,
		AuthorityYawDegrees);
	
	//判断玩家是否松开了摇杆
	if (RuntimeData.DesiredWorldDirection.IsNearlyZero())
	{
		RuntimeData.DesiredLocalMoveAngle = 0.0f;

		RuntimeData.DesiredMoveDirection = EPlayerMoveDirection::None;

		return;
	}

	RuntimeData.DesiredLocalMoveAngle = CalculateLocalMoveAngle(
		CharacterForward,
		RuntimeData.DesiredWorldDirection);

	RuntimeData.DesiredMoveDirection = QuantizeDirection(RuntimeData.DesiredLocalMoveAngle);

}

FVector FPlayerMovementParameterProcessor::CalculateWorldDirection(
	const FVector2D& MoveInput,
	float AuthorityYawDegrees) const
{
	const FRotator AuthorityYawRotation(
		0.0f,
		AuthorityYawDegrees,
		0.0f);

	const FRotationMatrix DirectionBasis(
		AuthorityYawRotation);

	const FVector BasisForward =
		DirectionBasis.GetUnitAxis(EAxis::X);

	const FVector BasisRight =
		DirectionBasis.GetUnitAxis(EAxis::Y);

	return (
		BasisForward * MoveInput.Y +
		BasisRight * MoveInput.X
		).GetSafeNormal2D();
}

float FPlayerMovementParameterProcessor::CalculateLocalMoveAngle(
	const FVector& CharacterForward,
	const FVector& WorldDirection) const
{
	const FVector FlatForward =
		CharacterForward.GetSafeNormal2D();

	const FVector FlatDirection =
		WorldDirection.GetSafeNormal2D();

	if (FlatForward.IsNearlyZero() ||
		FlatDirection.IsNearlyZero())
	{
		return 0.0f;
	}

	const float Dot = FMath::Clamp(
		FVector::DotProduct(
			FlatForward,
			FlatDirection),
		-1.0f,
		1.0f);

	const float CrossZ =
		FVector::CrossProduct(
			FlatForward,
			FlatDirection).Z;

	return FMath::RadiansToDegrees(
		FMath::Atan2(CrossZ, Dot));
}

EPlayerMoveDirection
FPlayerMovementParameterProcessor::QuantizeDirection(
	float LocalMoveAngle) const
{
	const float Angle =
		FRotator::NormalizeAxis(LocalMoveAngle);

	constexpr float HalfSectorAngle = 22.5f;

	if (Angle > -HalfSectorAngle &&
		Angle <= HalfSectorAngle)
	{
		return EPlayerMoveDirection::Forward;
	}

	if (Angle > 22.5f && Angle <= 67.5f)
	{
		return EPlayerMoveDirection::ForwardRight;
	}

	if (Angle > 67.5f && Angle <= 112.5f)
	{
		return EPlayerMoveDirection::Right;
	}

	if (Angle > 112.5f && Angle <= 157.5f)
	{
		return EPlayerMoveDirection::BackwardRight;
	}

	if (Angle > 157.5f || Angle <= -157.5f)
	{
		return EPlayerMoveDirection::Backward;
	}

	if (Angle > -157.5f && Angle <= -112.5f)
	{
		return EPlayerMoveDirection::BackwardLeft;
	}

	if (Angle > -112.5f && Angle <= -67.5f)
	{
		return EPlayerMoveDirection::Left;
	}

	return EPlayerMoveDirection::ForwardLeft;
}
