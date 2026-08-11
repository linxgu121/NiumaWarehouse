#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Container/NiumaSpatialContainerConfig.h"
#include "NiumaWarehouse/Container/NiumaSpatialContainerState.h"
#include "NiumaWarehouse/Definitions/NiumaItemSpatialDefinitionResolver.h"
#include "NiumaWarehouse/Result/NiumaWarehouseOperationResult.h"
#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"

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
     * 使用持久化 State 初始化容器并重建占用缓存。
     * 每条 Placement 都会重新经过定义解析、类型、
     * 旋转、边界和碰撞检查。
     * 只有全部 Placement 重建成功后才提交正式容器；
     * 失败时当前容器保持原样。
    */
    bool TryInitializeFromState(
        const FNiumaSpatialContainerConfig& InConfig,
        const FNiumaSpatialContainerState& InState,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FString* OutError = nullptr);

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

    /**
    * 判断一条 Placement 是否可以放入当前容器。
    *
    * 只进行验证和查询，不修改容器状态。
    */
    ENiumaWarehouseOperationResult CanPlace(
        const FNiumaSpatialItemPlacement& Placement,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FString* OutError = nullptr) const;

    /**
    * 按确定性顺序寻找物品的第一个合法放置位置。
    * 搜索顺序：
    * Y 从上到下，X 从左到右，
    * 方向依次为 0、90、180、270 度。
    * 本函数只查询，不修改容器状态。
    * 失败时不修改 OutPlacement。
    */
    ENiumaWarehouseOperationResult FindFirstValidPlacement(
        const FNiumaItemInstance& Item,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FNiumaSpatialItemPlacement& OutPlacement,
        FString* OutError = nullptr) const;

    /**
    * 把一条合法 Placement 原子提交到容器。
    * 成功时：
    * - Placements 增加一条记录
    * - Footprint 对应格写入新 Placement 下标
    * - Revision 只增加一次
    * 失败时容器保持原样。
    */
    ENiumaWarehouseOperationResult TryPlace(
        const FNiumaSpatialItemPlacement& Placement,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FString* OutError = nullptr);

    /**
    * 根据 InstanceId 原子移除一条 Placement。
    * 成功时：
    * - 删除对应 Placement
    * - 释放它占用的全部逻辑格
    * - 修正后续 Placement 的缓存下标
    * - Revision 只增加一次
    * 失败时容器保持原样。
    */
    ENiumaWarehouseOperationResult TryRemove(const FGuid& InstanceId,FString* OutError = nullptr);

    /**
    * 把指定物品实例移动到新的二维原点。
    * 方向和物品数据保持不变。
    * 成功修改位置时 Revision 增加一次。
    * 失败时保持原位置。
    */
    ENiumaWarehouseOperationResult TryMove(
        const FGuid& InstanceId,
        FIntPoint NewOrigin,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FString* OutError = nullptr);

    /**
     * 把指定物品实例旋转到目标方向。
     * Origin 和物品实例数据保持不变。
     * 成功修改方向时 Revision 增加一次。
     * 失败时保持原方向和原占用。
    */
    ENiumaWarehouseOperationResult TryRotate(
        const FGuid& InstanceId,
        ENiumaItemOrientation NewOrientation,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FString* OutError = nullptr);

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

    /**
    * 执行完整放置评估。
    * IgnoredPlacementIndex：
    * - INDEX_NONE：正常检测全部占用；
    * - 合法下标：碰撞检测时忽略该 Placement。
    * OutResolvedData 只在成功时写入。
    */
    ENiumaWarehouseOperationResult EvaluatePlacement(
        const FNiumaSpatialItemPlacement& Placement,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        int32 IgnoredPlacementIndex,
        FNiumaResolvedItemSpatialData* OutResolvedData,
        FString* OutError) const;

    /**
    * 根据 InstanceId 查询 Placement 下标。
    *
    * 找不到时返回 INDEX_NONE。
    */
    int32 FindPlacementIndexByInstanceId(const FGuid& InstanceId) const;

    /**
    * 原子替换指定下标的 Placement。
    * 用于复用移动和旋转的释放旧占用、
    * 写入新占用及提交逻辑。
    */
    ENiumaWarehouseOperationResult TryReplacePlacementAt(
        int32 PlacementIndex,
        const FNiumaSpatialItemPlacement& CandidatePlacement,
        const INiumaItemSpatialDefinitionResolver& Resolver,
        FString* OutError);

    /**
    * 使用已经解析完成的空间数据检查 Placement。
    * 调用前必须已经验证：
    * - 容器已初始化；
    * - Placement 和 Item 结构合法；
    * - IgnoredPlacementIndex 合法或为 INDEX_NONE。
    * 本函数负责验证解析数据、类型、旋转许可、
    * 边界和碰撞检查。
    */
    ENiumaWarehouseOperationResult
        EvaluateResolvedPlacement(
            const FNiumaSpatialItemPlacement& Placement,
            const FNiumaResolvedItemSpatialData& ResolvedData,
            int32 IgnoredPlacementIndex,
            FString* OutError) const;
};
