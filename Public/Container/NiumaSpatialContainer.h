#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Container/NiumaSpatialContainerConfig.h"
#include "NiumaWarehouse/Container/NiumaSpatialContainerState.h"

/**
 * 通用二维空间容器核心。
 *
 * 按值拥有规则、存档状态和可重建的占用缓存。
 * 不依赖 UObject、World、UI 或 Asset Manager。
 */
class NIUMA_API FNiumaSpatialContainer final
{
public:
	FNiumaSpatialContainer() = default;

    /**
    * 使用合法 Config 初始化一个全新的空容器。
    *
    * 成功后：
    * - State 为空且 Revision = 0
    * - OccupancyCache 全部为 INDEX_NONE
    * - IsInitialized() 返回 true
    *
    * 失败时保持容器原样。
    */
    bool TryInitializeEmpty(const FNiumaSpatialContainerConfig& InConfig, FString* OutError = nullptr);

    /**
     * 当前容器是否已经成功初始化。
     */
    bool IsInitialized() const;

    /**
    * 获取只读运行时配置。
    */
    const FNiumaSpatialContainerConfig& GetConfig() const;

    /**
     * 获取只读可持久化状态。
     */
    const FNiumaSpatialContainerState& GetState() const;

private:
    /**
     * 当前容器使用的不可随意修改规则。
     */
    FNiumaSpatialContainerConfig Config;

    /**
    * 当前容器的可持久化状态。
    */
    FNiumaSpatialContainerState State;

    /**
     * 二维占用缓存。
     *
     * 每个元素保存 Placements 下标；
     * INDEX_NONE 表示该逻辑格为空。
     */
    TArray<int32> OccupancyCache;

    /**
    * 只有完整初始化成功后才能设为 true。
    */
    bool bInitialized = false;

};
