#include "NiumaWarehouse/Remote/NiumaWarehouseHttpResponseMapper.h"

#include "NiumaWarehouse/Remote/NiumaWarehouseJsonConverter.h"

namespace
{
    constexpr int32 SnapshotSuccessStatusCode = 200;
    constexpr int32 MinimumErrorStatusCode = 400;
    constexpr int32 MaximumErrorStatusCode = 599;
}

FNiumaWarehouseSnapshotResult FNiumaWarehouseHttpResponseMapper::MapResponse(
    int32 HttpStatusCode,
    const FString& ResponseBody)
{
    if (HttpStatusCode <= 0)
    {
        return FNiumaWarehouseSnapshotResult::
            MakeTransportFailure(TEXT("仓库响应缺少有效 HTTP 状态码"));
    }

    FNiumaWarehouseSnapshotResponseDto ParsedResponse;
    FString ParseError;

    if (!FNiumaWarehouseJsonConverter::
        TryParseSnapshotResponse(
            ResponseBody,
            ParsedResponse,
            &ParseError))
    {
        return FNiumaWarehouseSnapshotResult::
            MakeProtocolFailure(
                HttpStatusCode,
                MoveTemp(ParseError));
    }

    if (ParsedResponse.bSuccess)
    {
        if (HttpStatusCode !=
            SnapshotSuccessStatusCode)
        {
            return FNiumaWarehouseSnapshotResult::
                MakeProtocolFailure(
                    HttpStatusCode,
                    TEXT("仓库业务成功但 HTTP 状态码不是 200"));
        }

        return FNiumaWarehouseSnapshotResult::
            MakeSuccess(
                HttpStatusCode,
                MoveTemp(ParsedResponse.Data));
    }

    if (HttpStatusCode <
        MinimumErrorStatusCode ||
        HttpStatusCode >
        MaximumErrorStatusCode)
    {
        return FNiumaWarehouseSnapshotResult::
            MakeProtocolFailure(
                HttpStatusCode,
                TEXT("仓库业务失败但 HTTP 状态码不是错误状态"));
    }

    return FNiumaWarehouseSnapshotResult::
        MakeBusinessFailure(
            HttpStatusCode,
            MoveTemp(ParsedResponse.Code),
            MoveTemp(ParsedResponse.Message));
}
