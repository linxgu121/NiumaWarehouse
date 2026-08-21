#include "NiumaTPC/Character/NiumaCharacter.h"

#include "Templates/UniquePtr.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "NiumaTPC/Character/States/Types/PlayerStateType.h"
#include "NiumaTPC/Character/Animation/NiumaAnimationFacadeComponent.h"
#include "NiumaTPC/Character/StateMachine/StateBase.h"
#include "NiumaTPC/Character/States/Locomotion/PlayerIdleState.h"
#include "NiumaTPC/Character/States/PlayerBaseState.h"
#include "NiumaTPC/Character/Config/Animation/PlayerLocomotionAnimationData.h"


ANiumaCharacter::ANiumaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建动画门面组件
	AnimationFacade = CreateDefaultSubobject<UNiumaAnimationFacadeComponent>(TEXT("AnimationFacade"));
}

void ANiumaCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!InitializePlayerStateSystem())
	{
		UE_LOG(LogTemp, Error, TEXT("NiumaCharacter玩家状态初始化异常"));

		// 清理可能已经创建的部分状态
		ShutdownPlayerStateSystem();
	}
}

void ANiumaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LocomotionIntentProcessor.Update(RuntimeData, LocomotionIntent, LocomotionPermissions);

	MovementParameterProcessor.Update(
		RuntimeData,
		GetControlRotation().Yaw,
		GetActorForwardVector());


	// 调用状态机的逻辑更新方法，传入每帧的时间增量
	if(FStateBase* CurrentState = StateMachine.GetCurrentState())
	{
		CurrentState->LogicUpdate(DeltaTime);
	}

	/*
	* 这里暂时只调用 LogicUpdate()
	* 不要在普通 Tick() 中直接调用 PhysicsUpdate()。
	* UE没有和 Unity FixedUpdate() 完全对应的角色函数
	* 物理移动以后交给 UCharacterMovementComponent 或明确的物理 Tick 阶段。
	*/
	
	//状态处理完本帧意图后再清除
	RuntimeData.ResetFrameIntents();
}

void ANiumaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ShutdownPlayerStateSystem();

	Super::EndPlay(EndPlayReason);
}

/// <summary>
/// 初始化状态系统
/// </summary>
/// <returns></returns>
bool ANiumaCharacter::InitializePlayerStateSystem()
{
	if (!IsValid(LocomotionAnimationData.Get()))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[NiumaCharacter] "
				"LocomotionAnimationData未绑定"));

		return false;
	}

	StateContext.Initialize(
		this,
		GetCharacterMovement(),
		&RuntimeData,
		AnimationFacade.Get(),
		LocomotionAnimationData.Get());

	if (!StateContext.IsReady())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[NiumaCharacter] "
				"StateContext缺少必要依赖"));

		return false;
	}

	if (!BuildStateRegistry())
	{
		return false;
	}

	FPlayerBaseState* InitialState = StateRegistry.FindState(EPlayerStateType::Idle);

	if (!InitialState)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[NiumaCharacter] "
				"找不到初始Idle状态"));

		return false;
	}

	StateMachine.Initialize(InitialState);

	return StateMachine.GetCurrentState() == InitialState;
}

/// <summary>
/// 状态构建
/// </summary>
bool ANiumaCharacter::BuildStateRegistry()
{
	if (!StateRegistry.RegisterState(MakeUnique<FPlayerIdleState>(StateContext)))
	{
		UE_LOG(LogTemp, Error, TEXT("NiumaCharacter registry Idle state fall"));

		return false;
	}

	// 后续状态只在这里加入：

	return true;
}

/// <summary>
/// 关停玩家整套状态系统
/// </summary>
void ANiumaCharacter::ShutdownPlayerStateSystem()
{
	//状态机整体关停
	StateMachine.Shutdown();
	//清空全局状态注册表
	StateRegistry.Reset();
	//清空玩家上下文缓存
	StateContext.Reset();
}

void ANiumaCharacter::ReceiveLocomotionIntent(const FPlayerLocomotionIntent& InIntent)
{
	LocomotionIntent = InIntent;

	// 接收边界负责保证输入长度不超过1。
	LocomotionIntent.MoveInput =
		InIntent.MoveInput.GetClampedToMaxSize(1.0f);
}

/// <summary>
/// 失去控制时清空意图
/// </summary>
void ANiumaCharacter::UnPossessed()
{
	Super::UnPossessed();

	ReceiveLocomotionIntent(FPlayerLocomotionIntent{});
}

FStateMachine& ANiumaCharacter::GetStateMachine()
{
	return StateMachine;
}

const FStateMachine& ANiumaCharacter::GetStateMachine() const
{
	return StateMachine;
}

FPlayerRuntimeData& ANiumaCharacter::GetRuntimeData()
{
	return RuntimeData;
}

const FPlayerRuntimeData& ANiumaCharacter::GetRuntimeData() const
{
	return RuntimeData;
}

UNiumaAnimationFacadeComponent* ANiumaCharacter::GetAnimationFacade() const
{
	return AnimationFacade;
}

FPlayerStateContext& ANiumaCharacter::GetStateContext()
{
	return StateContext;
}

const FPlayerStateContext& ANiumaCharacter::GetStateContext() const
{
	return StateContext;
}

FPlayerStateRegistry& ANiumaCharacter::GetStateRegistry()
{
	return StateRegistry;
}

const FPlayerStateRegistry& ANiumaCharacter::GetStateRegistry() const
{
	return StateRegistry;
}
