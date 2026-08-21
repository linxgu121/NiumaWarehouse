#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiumaTPC/Character/Animation/IPlayerAnimationFacade.h"

#include "NiumaAnimationFacadeComponent.generated.h"


class UAnimInstance;
class USkeletalMeshComponent;

UCLASS(ClassGroup = (Niuma), meta = (BlueprintSpawnableComponent))

class NIUMA_API UNiumaAnimationFacadeComponent final : public UActorComponent, public IPlayerAnimationFacade
{
	GENERATED_BODY()

public:
	UNiumaAnimationFacadeComponent();

	virtual float PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f) override;

	virtual void StopMontage(UAnimMontage* Montage, float BlendOutTime = 0.2f) override;

	virtual bool IsMontagePlaying(const UAnimMontage* Montage) const override;

	virtual float GetMontagePosition(const UAnimMontage* Montage) const override;

	virtual float GetMontageNormalizedTime(const UAnimMontage* Montage) const override;

	virtual void SetMontageEndedCallback(UAnimMontage* Montage, const FNiumaMontageEndedCallback& Callback) override;

	virtual void ClearMontageEndedCallback(UAnimMontage* Montage) override;
protected:
	/// <summary>
	/// 游戏运行初始化，查找组件、绑定事件、缓存引用
	/// </summary>
	virtual void BeginPlay() override;

	/// <summary>
	/// 对象销毁收尾，解绑事件、清空缓存、切断引用。
	/// </summary>
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UAnimInstance* GetAnimInstance() const;

	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> CallbackMontage = nullptr;

	FNiumaMontageEndedCallback MontageEndedCallback;
	
};

