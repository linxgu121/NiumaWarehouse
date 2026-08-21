#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "NiumaTPC/Character/RuntimeData/PlayerRuntimeData.h"
#include "NiumaTPC/Character/StateMachine/StateMachine.h"
#include "NiumaTPC/Character/States/PlayerStateContext.h"
#include "NiumaTPC/Character/States/Registry/PlayerStateRegistry.h"
#include "NiumaTPC/Character/RuntimeData/Locomotion/PlayerLocomotionIntent.h"
#include "NiumaTPC/Character/Arbitration/Locomotion/PlayerLocomotionIntentProcessor.h"
#include "NiumaTPC/Character/Input/Interfaces/IPlayerLocomotionIntentReceiver.h"
#include "NiumaTPC/Character/Processing/Locomotion/PlayerMovementParameterProcessor.h"

#include "NiumaCharacter.generated.h"

class UNiumaAnimationFacadeComponent;
class UNiumaPlayerLocomotionAnimationData;

UCLASS()
class NIUMA_API ANiumaCharacter : public ACharacter, public IPlayerLocomotionIntentReceiver
{
	
	GENERATED_BODY()

public:
	ANiumaCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void ReceiveLocomotionIntent(const FPlayerLocomotionIntent& InIntent) override;

	FStateMachine& GetStateMachine();
	const FStateMachine& GetStateMachine() const;

	FPlayerRuntimeData& GetRuntimeData();
	const FPlayerRuntimeData& GetRuntimeData() const;

	UNiumaAnimationFacadeComponent* GetAnimationFacade() const;	

	FPlayerStateContext& GetStateContext();
	
	const FPlayerStateContext& GetStateContext() const;

	FPlayerStateRegistry& GetStateRegistry();

	const FPlayerStateRegistry& GetStateRegistry() const;
protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void UnPossessed() override;
private:
	bool InitializePlayerStateSystem();
	bool BuildStateRegistry();
	void ShutdownPlayerStateSystem();

	FPlayerLocomotionIntent LocomotionIntent;

	FPlayerLocomotionPermissions LocomotionPermissions;

	FPlayerLocomotionIntentProcessor LocomotionIntentProcessor;

	FPlayerMovementParameterProcessor MovementParameterProcessor;


	FPlayerRuntimeData RuntimeData;

	// 外部只能看，不能改；蓝图能读，不能写
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Niuma|Animation",
		meta = (AllowPrivateAccess = true))
	TObjectPtr<UNiumaAnimationFacadeComponent> AnimationFacade = nullptr;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Niuma|Animation|Config",
		meta = (AllowPrivateAccess = true))
	TObjectPtr<UNiumaPlayerLocomotionAnimationData> LocomotionAnimationData = nullptr;

	FPlayerStateContext StateContext;

	// Registry拥有状态
	FPlayerStateRegistry StateRegistry;

	//StateMachine只观察Registry中的状态
	FStateMachine StateMachine;
	
	
};