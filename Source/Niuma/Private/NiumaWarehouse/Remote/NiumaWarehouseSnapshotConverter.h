#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Container/NiumaSpatialContainerConfig.h"
#include "NiumaWarehouse/Container/NiumaSpatialContainerState.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseRemoteDtos.h"

class UNiumaWarehouseDefinition;

/**
 * 把未经领域验证的网络快照转换为候选容器数据。
 *
 * 本类只负责协议数据到领域结构的转换：
 * - 不发送 HTTP
 * - 不加载物品资产
 * - 不重建 Occupancy
 * - 不修改正式仓库
 */
class FNiumaWarehouseSnapshotConverter final
{
public:
    /**
     * 原子转换仓库快照。
     *
     * 只有全部字段转换成功时，才同时修改
     * OutConfig 与 OutState；失败时两个输出保持原样。
     */
    static bool TryConvert(
        const FNiumaWarehouseSnapshotDto& Snapshot,
        const UNiumaWarehouseDefinition& WarehouseDefinition,
        int32 SupportedSchemaVersion,
        int32 ExpectedCatalogVersion,
        FNiumaSpatialContainerConfig& OutConfig,
        FNiumaSpatialContainerState& OutState,
        FString* OutError = nullptr);

private:
    FNiumaWarehouseSnapshotConverter() = delete;
};