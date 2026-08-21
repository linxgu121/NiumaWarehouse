#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"


#include "NiumaTPC/Character/Motion/PlayerFootPhase.h"
#include "NiumaTPC/Character/Motion/PlayerLocomotionType.h"
#include "NiumaTPC/Character/Motion/PlayerMoveDirection.h"

#include "PlayerLocomotionAnimationData.generated.h"

class UAnimMontage;

/*
* 单个起步动画及其附加参数
*/
USTRUCT(BlueprintType)
struct NIUMA_API FPlayerMoveStartAnimation
{
	GENERATED_BODY()

	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "MoveStart")
	/*
	* 动画片段
	*/
	TObjectPtr<UAnimMontage> Montage = nullptr;


	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "MoveStart",
		meta = (ClampMin = "0.01", UIMin = "0.01"))
	/*
	* 播放速度
	*/
	float PlayRate = 1.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "MoveStart")
	/*
	* 动画结束时的脚步相位
	*/
	EPlayerFootPhase EndFootPhase = EPlayerFootPhase::LeftFootDown;
};

/// <summary>
/// 某一种步态下的八方向起步动画
/// </summary>
USTRUCT(BlueprintType)
struct NIUMA_API FPlayerDirectionalMoveStartSet
{
	GENERATED_BODY()

public:
	const FPlayerMoveStartAnimation* Find(EPlayerMoveDirection Direction) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation Forward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation ForwardRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation BackwardRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation Backward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation BackwardLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MoveStart")
	FPlayerMoveStartAnimation ForwardLeft;
};

/*
* 玩家地面移动动画的静态配置资产
*/
UCLASS(BlueprintType)
class NIUMA_API UNiumaPlayerLocomotionAnimationData
	: public UDataAsset
{
	GENERATED_BODY()

public:
	const FPlayerMoveStartAnimation* FindMoveStartAnimation(
		EPlayerLocomotionState LocomotionState,
		EPlayerMoveDirection Direction) const;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Niuma|Locomotion|MoveStart")
	FPlayerDirectionalMoveStartSet WalkStarts;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Niuma|Locomotion|MoveStart")
	FPlayerDirectionalMoveStartSet JogStarts;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Niuma|Locomotion|MoveStart")
	FPlayerDirectionalMoveStartSet SprintStarts;
};
