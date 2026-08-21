#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "IPlayerLocomotionIntentReceiver.generated.h"

struct FPlayerLocomotionIntent;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPlayerLocomotionIntentReceiver : public UInterface
{
	GENERATED_BODY()
};

class NIUMA_API IPlayerLocomotionIntentReceiver
{
    GENERATED_BODY()

public:
    virtual void ReceiveLocomotionIntent(
        const FPlayerLocomotionIntent& InIntent) = 0;
};