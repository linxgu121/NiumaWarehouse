#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouseGrantDtos.generated.h"

/**
 * POST /api/v1/dev/warehouse/items/grant
 * 的请求 Body。
 *
 * 这是开发环境专用的网络协议 DTO，
 * 不是仓库领域对象，也不向蓝图公开。
 *
 * 账号 UUID 来自当前登录会话；
 * InstanceId、位置和朝向由 Java 服务端生成。
 */
USTRUCT()
struct NIUMA_API FNiumaWarehouseGrantRequestDto
{
	GENERATED_BODY()

    /**
     * 服务端权威物品定义 ID。
     */
    UPROPERTY()
    FString ItemDefinitionId;

    /**
     * 本次发放数量。
     *
     * 默认 0 是故意设置的非法值，
     * 防止未完整构造的 DTO 被意外发送。
     */
    UPROPERTY()
    int32 Count = 0;
};
