#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

#include "NiumaNetwork/Dto/NiumaAccountRemoteDtos.h"
#include "NiumaNetwork/Result/NiumaAccountLoginResult.h"

#if WITH_DEV_AUTOMATION_TESTS

using FNiumaAccountLoginRequestHandlerForTesting =
TFunction<void(
	const FNiumaAccountLoginRequestDto&,
	FNiumaAccountLoginCompleted)>;

namespace NiumaAccountHttpGatewayTestHook
{
	void SetLoginRequestHandler(
		FNiumaAccountLoginRequestHandlerForTesting
		InHandler);

	void ResetLoginRequestHandler();
}

#endif