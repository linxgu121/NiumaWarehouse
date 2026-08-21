#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotResult.h"

#if WITH_DEV_AUTOMATION_TESTS

using FNiumaWarehouseSnapshotRequestHandlerForTesting =
    TFunction<void(
        FString,
        FString,
        FNiumaWarehouseSnapshotCompleted)>;

/**
 * 参数依次为：
 * URL、Authorization Header、JSON Body、完成回调。
 */
using FNiumaWarehouseGrantRequestHandlerForTesting =
TFunction<void(
    FString Url,
    FString AuthorizationHeader,
    FString RequestBody,
    FNiumaWarehouseSnapshotCompleted Completion)>;

/**
 * 参数依次为：
 * URL、Authorization Header、JSON Body、完成回调。
 */
using FNiumaWarehouseRelocateRequestHandlerForTesting =
TFunction<void(
    FString Url,
    FString AuthorizationHeader,
    FString RequestBody,
    FNiumaWarehouseSnapshotCompleted Completion)>;

namespace NiumaWarehouseHttpGatewayTestHook
{
    void SetSnapshotRequestHandler(
        FNiumaWarehouseSnapshotRequestHandlerForTesting
            InHandler);

    void ResetSnapshotRequestHandler();

    void SetRelocateRequestHandler(
        FNiumaWarehouseRelocateRequestHandlerForTesting
        InHandler);

    void ResetRelocateRequestHandler();

    void SetGrantRequestHandler(
        FNiumaWarehouseGrantRequestHandlerForTesting InHandler);

    void ResetGrantRequestHandler();
}

#endif
