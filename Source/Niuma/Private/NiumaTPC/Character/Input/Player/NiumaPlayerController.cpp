#include "NiumaTPC/Character/Input/Player/NiumaPlayerController.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

#include "NiumaTPC/Character/Input/Interfaces/IPlayerLocomotionIntentReceiver.h"


void ANiumaPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!IsLocalPlayerController())
    {
        return;
    }

    InstallMappingContexts();
    BindInputActions();
}

void ANiumaPlayerController::OnPossess(
    APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 新Pawn立即收到当前完整快照。
    PublishLocomotionIntent();
}

void ANiumaPlayerController::OnUnPossess()
{
    // 在GetPawn()失效前把空快照发给旧Pawn。
    ResetLocomotionIntent();
    PublishLocomotionIntent();

    Super::OnUnPossess();
}

void ANiumaPlayerController::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    RemoveMappingContexts();

    Super::EndPlay(EndPlayReason);
}
 

void ANiumaPlayerController::
InstallMappingContexts()
{
    if (bMappingContextsInstalled)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();

    if (!LocalPlayer)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error,TEXT("[NiumaPlayerController] ""找不到Enhanced Input Subsystem"));

        return;
    }

    for (const TObjectPtr<UInputMappingContext>& MappingContext : DefaultMappingContexts)
    {
        if (MappingContext)
        {
            Subsystem->AddMappingContext(MappingContext.Get(), 0);
        }
    }

    bMappingContextsInstalled = true;
}

void ANiumaPlayerController::RemoveMappingContexts()
{
    if (!bMappingContextsInstalled)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();

    if (LocalPlayer)
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

        if (Subsystem)
        {
            for (
                const TObjectPtr<UInputMappingContext>& MappingContext : DefaultMappingContexts)
            {
                if (MappingContext)
                {
                    Subsystem->RemoveMappingContext(
                        MappingContext.Get());
                }
            }
        }
    }

    bMappingContextsInstalled = false;
}



void ANiumaPlayerController::BindInputActions()
{
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);

    if (!EnhancedInput)
    {
        UE_LOG(LogTemp,Error,TEXT( "[NiumaPlayerController] ""InputComponent不是EnhancedInputComponent"));

        return;
    }

    if (MoveAction)
    {
        EnhancedInput->BindAction(
            MoveAction,
            ETriggerEvent::Triggered,
            this,
            &ANiumaPlayerController::
            HandleMoveInput);

        EnhancedInput->BindAction(
            MoveAction,
            ETriggerEvent::Completed,
            this,
            &ANiumaPlayerController::
            StopMoveInput);

        EnhancedInput->BindAction(
            MoveAction,
            ETriggerEvent::Canceled,
            this,
            &ANiumaPlayerController::
            StopMoveInput);
    }

    if (WalkAction)
    {
        EnhancedInput->BindAction(
            WalkAction,
            ETriggerEvent::Started,
            this,
            &ANiumaPlayerController::
            StartWalkInput);

        EnhancedInput->BindAction(
            WalkAction,
            ETriggerEvent::Completed,
            this,
            &ANiumaPlayerController::
            StopWalkInput);

        EnhancedInput->BindAction(
            WalkAction,
            ETriggerEvent::Canceled,
            this,
            &ANiumaPlayerController::
            StopWalkInput);
    }

    if (SprintAction)
    {
        EnhancedInput->BindAction(
            SprintAction,
            ETriggerEvent::Started,
            this,
            &ANiumaPlayerController::
            StartSprintInput);

        EnhancedInput->BindAction(
            SprintAction,
            ETriggerEvent::Completed,
            this,
            &ANiumaPlayerController::
            StopSprintInput);

        EnhancedInput->BindAction(
            SprintAction,
            ETriggerEvent::Canceled,
            this,
            &ANiumaPlayerController::
            StopSprintInput);
    }
}



void ANiumaPlayerController::HandleMoveInput(const FInputActionValue& Value)
{
    LocomotionIntent.MoveInput = Value.Get<FVector2D>().GetClampedToMaxSize(1.0f);

    PublishLocomotionIntent();
}

void ANiumaPlayerController::StopMoveInput()
{
    LocomotionIntent.MoveInput = FVector2D::ZeroVector;

    PublishLocomotionIntent();
}

void ANiumaPlayerController::StartWalkInput()
{
    LocomotionIntent.bWalkHeld = true;

    PublishLocomotionIntent();
}

void ANiumaPlayerController::StopWalkInput()
{
    LocomotionIntent.bWalkHeld = false;

    PublishLocomotionIntent();
}

void ANiumaPlayerController::StartSprintInput()
{
    LocomotionIntent.bSprintHeld = true;

    PublishLocomotionIntent();
}

void ANiumaPlayerController::StopSprintInput()
{
    LocomotionIntent.bSprintHeld = false;

    PublishLocomotionIntent();
}



void ANiumaPlayerController::PublishLocomotionIntent()
{
    APawn* ControlledPawn = GetPawn();

    if (!ControlledPawn)
    {
        return;
    }

    IPlayerLocomotionIntentReceiver* Receiver =
        Cast<IPlayerLocomotionIntentReceiver>(
            ControlledPawn);

    if (!Receiver)
    {
        return;
    }

    Receiver->ReceiveLocomotionIntent(
        LocomotionIntent);
}

void ANiumaPlayerController::ResetLocomotionIntent()
{
    LocomotionIntent =  FPlayerLocomotionIntent{};
}

