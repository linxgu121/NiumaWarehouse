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

    /**
    * 查询指定逻辑格保存的 Placement 下标。
    * 返回 true 表示查询成功：
    * - OutPlacementIndex == INDEX_NONE：该格为空
    * - 其他值：对应 State.Placements 中的下标
    * 返回 false 表示容器未初始化、坐标越界或内部缓存异常。
    * 失败时不修改 OutPlacementIndex。
    */
    bool TryGetPlacementIndexAt(
        const FIntPoint& Cell,
        int32& OutPlacementIndex,
        FString* OutError = nullptr) const;

private:
    /**
    * 判断逻辑格是否位于当前容器范围内。
    */
    bool IsCellInBounds(const FIntPoint& Cell) const;

    /**
    * 把合法的二维坐标转换为 OccupancyCache 下标。
    *
    * 调用者必须先保证 Cell 位于容器范围内。
    */
    int32 ToFlatIndexUnchecked(const FIntPoint& Cell) const;

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
