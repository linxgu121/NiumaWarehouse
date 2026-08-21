#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotResult.h"

/**
 * 把 HTTP 状态码与响应正文映射为仓库快照结果。
 *
 * 同时校验 HTTP 状态与 JSON 业务语义是否一致。
 */
class FNiumaWarehouseHttpResponseMapper final
{
public:
    static FNiumaWarehouseSnapshotResult MapResponse(
        int32 HttpStatusCode,
        const FString& ResponseBody);

private:
    FNiumaWarehouseHttpResponseMapper() = delete;
};
