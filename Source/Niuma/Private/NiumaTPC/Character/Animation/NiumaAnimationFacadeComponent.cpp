#include "NiumaTPC/Character/Animation/NiumaAnimationFacadeComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UNiumaAnimationFacadeComponent::UNiumaAnimationFacadeComponent()
{
	//关闭该动画门面组件的自动帧刷新
	PrimaryComponentTick.bCanEverTick = false;
}

/// <summary>
/// 初始化动画门面组件
/// </summary>
void UNiumaAnimationFacadeComponent::BeginPlay()
{
	// 调用父类 UActorComponent::BeginPlay() 的原始实现
	// UE 的组件初始化是链式的。父类 BeginPlay 里可能注册了 Tick、绑定了委托、或者做了生命周期标记。
	// 如果不写这行，父类的初始化逻辑就被截断了，可能导致组件不工作。
	Super::BeginPlay();

	// 把 GetOwner() 返回的 AActor* 安全地转换成 ACharacter*
	//Cast<> 内部用了 UE 的反射系统（StaticClass() 比较），类型不匹配时返回 nullptr
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		//ACharacter 的快捷方法，返回它的骨骼网格体组件
		//ACharacter 内部固定有一个 Mesh 组件
		//GetMesh() 就是读这个指针。动画蒙太奇（PlayAnimMontage）最终要播到这个 Mesh 的 AnimInstance 上
		SkeletalMeshComponent = OwnerCharacter->GetMesh();
	}

	if(!SkeletalMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[NiumaAnimationFacade] Owner not Character or dont have SkeletalMesh"));
	}

}

float UNiumaAnimationFacadeComponent::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage || PlayRate <= 0.0f)
	{
		return 0.0f;
	}

	// 获取 SkeletalMeshComponent 的 AnimInstance(获取动画实例)
	UAnimInstance* AnimInstance = GetAnimInstance();

	if (!AnimInstance)
	{
		return 0.0f;
	}

	// 调用引擎原生接口播放
	return AnimInstance->Montage_Play(Montage, PlayRate);
}

void UNiumaAnimationFacadeComponent::StopMontage(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();

	if (!AnimInstance)
	{
		return;
	}

	//停止指定蒙太奇，支持淡出平滑过渡，避免动画生硬截断卡顿
	AnimInstance->Montage_Stop(FMath::Max(0.0f, BlendOutTime), Montage);
}

bool UNiumaAnimationFacadeComponent::IsMontagePlaying(const UAnimMontage* Montage) const
{
	if (!Montage)
	{
		return false;
	}
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	//调用引擎原生接口判断指定蒙太奇是否正在播放
	return AnimInstance->Montage_IsPlaying(Montage);
}

float UNiumaAnimationFacadeComponent::GetMontagePosition(const UAnimMontage* Montage) const
{
	if (!Montage)
	{
		return 0.0f;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance)
	{
		return 0.0f;
	}

	//调用引擎原生接口获取指定蒙太奇的播放位置（单位：秒）
	return AnimInstance->Montage_GetPosition(Montage);
}

float UNiumaAnimationFacadeComponent::GetMontageNormalizedTime(const UAnimMontage* Montage) const
{
	if (!Montage)
	{
		return 0.0f;
	}

	const float PlayLength = Montage->GetPlayLength();

	if (PlayLength <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	//调用 GetMontagePosition 获取当前播放位置，并除以总长度，得到归一化时间（0.0~1.0）
	return FMath::Clamp(GetMontagePosition(Montage) / PlayLength, 0.0f, 1.0f);
}

/// <summary>
/// 获取当前 SkeletalMeshComponent 的动画实例
/// </summary>
/// <returns>UAnimInstance 指针，如果 SkeletalMeshComponent 不存在则返回 nullptr</returns>
UAnimInstance* UNiumaAnimationFacadeComponent::GetAnimInstance() const
{
	if (!SkeletalMeshComponent)
	{
		return nullptr;
	}

	//调用 SkeletalMeshComponent 的 GetAnimInstance() 获取动画实例
	return SkeletalMeshComponent->GetAnimInstance();
}

void UNiumaAnimationFacadeComponent::SetMontageEndedCallback(UAnimMontage* Montage, const FNiumaMontageEndedCallback& Callback)
{
	if(!Montage || !Callback.IsBound())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();

	if (!AnimInstance || !AnimInstance->Montage_IsActive(Montage))
	{
		return;
	}

	if(CallbackMontage)
	{
		ClearMontageEndedCallback(CallbackMontage.Get());
	}

	CallbackMontage = Montage;
	MontageEndedCallback = Callback;

	FOnMontageEnded EngineCallback;

	EngineCallback.BindUObject(this, &UNiumaAnimationFacadeComponent::HandleMontageEnded);

	AnimInstance->Montage_SetEndDelegate(EngineCallback, Montage);

	
}

void UNiumaAnimationFacadeComponent::ClearMontageEndedCallback(UAnimMontage* Montage)
{
	if(!Montage || CallbackMontage != Montage)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();
	if(AnimInstance)
	{
		// 清除引擎的蒙太奇结束委托
		FOnMontageEnded EngineCallback;

		AnimInstance->Montage_SetEndDelegate(EngineCallback, Montage);
	}

	MontageEndedCallback.Unbind();
	CallbackMontage = nullptr;
	
	
}

void UNiumaAnimationFacadeComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CallbackMontage != Montage)
	{
		return;
	}

	//
	FNiumaMontageEndedCallback Callback = MontageEndedCallback;

	// 必须在执行外部回调前清空
	MontageEndedCallback.Unbind();
	CallbackMontage = nullptr;

	//UE 委托自带安全执行方法：
	//先判断委托是否绑定了有效函数；
	//已绑定 → 执行回调，传入参数；
	//未绑定 → 直接跳过，不会空调用、不会崩溃。
	Callback.ExecuteIfBound(bInterrupted);
}

void UNiumaAnimationFacadeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CallbackMontage)
	{
		ClearMontageEndedCallback(
			CallbackMontage.Get());
	}

	//调用父类的销毁收尾逻辑
	Super::EndPlay(EndPlayReason);
}