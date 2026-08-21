#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouseRelocateDtos.generated.h"
/**
 * POST /api/v1/game/warehouse/items/relocate
 * 的请求 Body。
 *
 * 这是网络协议 DTO，不是仓库领域对象。
 * 发送前必须由 Subsystem 验证并构造。
 */
USTRUCT()
struct NIUMA_API FNiumaWarehouseRelocateRequestDto
{
    GENERATED_BODY()

    /**
     * 需要重定位的物品实例 UUID。
     */
    UPROPERTY()
    FString InstanceId;

    /**
     * 目标逻辑格 X 坐标。
     */
    UPROPERTY()
    int32 OriginX = -1;

    /**
     * 目标逻辑格 Y 坐标。
     */
    UPROPERTY()
    int32 OriginY = -1;

    /**
     * 目标朝向，只允许 0、90、180、270。
     */
    UPROPERTY()
    int32 OrientationDegrees = -1;

    /**
     * 客户端当前持有的仓库 Revision。
     *
     * 服务端只会在版本一致时接受命令。
     */
    UPROPERTY()
    int64 ExpectedRevision = -1;
};
