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

    Response.FinalOrigin = Placement.Origin;

    Response.FinalOrientation = Placement.Orientation;

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

