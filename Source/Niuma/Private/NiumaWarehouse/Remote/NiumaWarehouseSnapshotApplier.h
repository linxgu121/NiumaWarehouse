#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Container/NiumaSpatialContainer.h"
#include "NiumaWarehouse/Container/NiumaSpatialContainerConfig.h"
#include "NiumaWarehouse/Container/NiumaSpatialContainerState.h"
#include "NiumaWarehouse/Definitions/NiumaItemSpatialDefinitionResolver.h"

/**
 * 把已经转换完成的网络快照应用到正式仓库。
 *
 * 本类负责：
 * - 创建候选空间容器
 * - 重新解析物品定义
 * - 重建 OccupancyCache
 * - 检查类型、旋转、边界和碰撞
 * - 成功后原子替换正式仓库
 */
class FNiumaWarehouseSnapshotApplier final
{
public:
    /**
     * 原子应用候选仓库快照。
     *
     * 成功时替换 InOutWarehouse；
     * 失败时 InOutWarehouse 保持原样。
     */
    static bool TryApply(
        const FNiumaSpatialContainerConfig& Config,
        const FNiumaSpatialContainerState& State,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FNiumaSpatialContainer& InOutWarehouse,
        FString* OutError = nullptr);

private:
    FNiumaWarehouseSnapshotApplier() = delete;
};
