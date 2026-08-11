#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"
#include "Misc/Guid.h"

#include "NiumaWarehouse/Result/NiumaWarehouseOperationResult.h"
#include "NiumaWarehouse/Type/NiumaItemOrientation.h"

#include "NiumaWarehouseOperationResponse.generated.h"

struct FNiumaSpatialItemPlacement;


/**
 * 仓库业务操作的结构化响应。
 */
USTRUCT(BlueprintType)
struct NIUMA_API FNiumaWarehouseOperationResponse
{
	GENERATED_BODY()

public:
    /**
    * 从一条合法 Placement 创建成功响应。
    * 如果 Placement 非法，则返回 InternalError。
    */
    static FNiumaWarehouseOperationResponse MakeSuccess(
        const FNiumaSpatialItemPlacement& Placement);

    /**
     * 创建失败响应。
     * None 和 Success 不是失败结果；
     * 如果误传，将转换为 InternalError。
     */
    static FNiumaWarehouseOperationResponse MakeFailure(
        ENiumaWarehouseOperationResult InResult,
        const FString& InErrorMessage);

    bool IsSuccess() const;


    UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|Response")
    ENiumaWarehouseOperationResult Result = ENiumaWarehouseOperationResult::None;

    /**
     * 成功操作涉及的物品实例。
     * 失败时保持无效 GUID。
     */
    UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|Response")
    FGuid InstanceId;

    /**
     * 成功放置后的最终二维原点。
     */
    UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|Response")
    FIntPoint FinalOrigin = FIntPoint::ZeroValue;

    /**
     * 成功放置后的最终方向。
     */
    UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|Response")
    ENiumaItemOrientation FinalOrientation = ENiumaItemOrientation::Degree0;

    /**
     * 开发期诊断信息。
     * 最终 UI 应根据 Result 映射本地化文本，
     * 不应直接把该字符串作为玩家提示。
     */
    UPROPERTY(BlueprintReadOnly,Category = "Niuma|Warehouse|Response")
    FString ErrorMessage;
};


