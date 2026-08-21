#include "NiumaWarehouse/Result/NiumaWarehouseOperationResponse.h"

#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"

FNiumaWarehouseOperationResponse FNiumaWarehouseOperationResponse::MakeSuccess(
    const FNiumaSpatialItemPlacement& Placement)
{
    FString PlacementError;

    if (!Placement.IsValid(&PlacementError))
    {
        return MakeFailure(
            ENiumaWarehouseOperationResult::InternalError,
            FString::Printf(
                TEXT("无法从非法 Placement 创建成功响应：%s"),
                *PlacementError));
    }

    FNiumaWarehouseOperationResponse Response;

    Response.Result = ENiumaWarehouseOperationResult::Success;

    Response.InstanceId = Placement.Item.InstanceId;

    Response.bHasFinalPlacement = true;

    Response.FinalOrigin = Placement.Origin;

    Response.FinalOrientation = Placement.Orientation;

    Response.ErrorMessage.Reset();

    return Response;
}

FNiumaWarehouseOperationResponse FNiumaWarehouseOperationResponse::MakeRemovalSuccess(
    const FGuid& InInstanceId)
{
    if (!InInstanceId.IsValid())
    {
        return MakeFailure(
            ENiumaWarehouseOperationResult::InternalError,
            TEXT("无法使用无效 InstanceId 创建移除成功响应"));
    }

    FNiumaWarehouseOperationResponse Response;

    Response.Result = ENiumaWarehouseOperationResult::Success;

    Response.InstanceId = InInstanceId;

    /*
     * 移除后不存在最终 Placement。
     * Origin 与 Orientation 保持默认值，
     * 调用方必须先检查 bHasFinalPlacement。
     */
    Response.bHasFinalPlacement = false;

    Response.ErrorMessage.Reset();

    return Response;
}

FNiumaWarehouseOperationResponse FNiumaWarehouseOperationResponse::MakeFailure(
    ENiumaWarehouseOperationResult InResult,
    const FString& InErrorMessage)
{
    FNiumaWarehouseOperationResponse Response;

    if (InResult == ENiumaWarehouseOperationResult::None ||
        InResult == ENiumaWarehouseOperationResult::Success)
    {
        Response.Result = ENiumaWarehouseOperationResult::InternalError;

        Response.ErrorMessage = TEXT("创建失败响应时传入了非失败结果");

        return Response;
    }

    Response.Result = InResult;

    Response.ErrorMessage =
        InErrorMessage.IsEmpty()
        ? TEXT("仓库操作失败")
        : InErrorMessage;

    return Response;
}

bool FNiumaWarehouseOperationResponse::IsSuccess() const
{
    return Result == ENiumaWarehouseOperationResult::Success;
}

