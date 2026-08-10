#include "NiumaWarehouse/Container/NiumaSpatialContainer.h"

namespace
{
    void SetSpatialContainerError(FString* OutError,const TCHAR* Message)
    {
        if (OutError != nullptr)
        {
            *OutError = Message;
        }
    }
}

bool FNiumaSpatialContainer::TryInitializeEmpty(
    const FNiumaSpatialContainerConfig& InConfig,
    FString* OutError)
{
    if (bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器已经初始化，不能重复初始化"));

        return false;
    }

    FString ConfigError;

    if (!InConfig.IsValid(&ConfigError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(TEXT("空间容器 Config 无效：%s"), *ConfigError);
        }

        return false;
    }

    FNiumaSpatialContainerConfig CandidateConfig = InConfig;

    FNiumaSpatialContainerState CandidateState;

    FString StateError;

    if (!CandidateState.IsStructurallyValid(&StateError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("初始空 State 无效：%s"),
                *StateError);
        }

        return false;
    }

    // 先提升为 int64 再乘，防止 int32 乘法提前溢出。
    const int64 CellCount =
        static_cast<int64>(CandidateConfig.Width) *
        static_cast<int64>(CandidateConfig.Height);

    if (CellCount <= 0 || CellCount > static_cast<int64>(MAX_int32))
    {
        SetSpatialContainerError(
            OutError,
            TEXT("空间容器逻辑格总数超出TArray<int32> 可表示范围"));

        return false;
    }

    TArray<int32> CandidateOccupancy;

    CandidateOccupancy.Init(INDEX_NONE,static_cast<int32>(CellCount));

    // 所有验证和内存准备成功后，才提交正式成员。
    Config = MoveTemp(CandidateConfig);
    State = MoveTemp(CandidateState);
    OccupancyCache = MoveTemp(CandidateOccupancy);
    bInitialized = true;

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}

bool FNiumaSpatialContainer::TryGetPlacementIndexAt(
    const FIntPoint& Cell,
    int32& OutPlacementIndex,
    FString* OutError) const
{
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError, TEXT("空间容器尚未初始化"));

        return false;
    }

    if (!IsCellInBounds(Cell))
    {
        SetSpatialContainerError(OutError,TEXT("查询坐标超出空间容器范围"));

        return false;
    }

    const int32 FlatIndex = ToFlatIndexUnchecked(Cell);

    if (!OccupancyCache.IsValidIndex(FlatIndex))
    {
        SetSpatialContainerError(OutError,TEXT("空间容器占用缓存与配置不一致"));

        return false;
    }

    // 所有检查完成后才修改输出参数。
    OutPlacementIndex = OccupancyCache[FlatIndex];

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}

bool FNiumaSpatialContainer::IsCellInBounds(const FIntPoint& Cell) const
{
    return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < Config.Width && Cell.Y < Config.Height;
}

int32 FNiumaSpatialContainer::ToFlatIndexUnchecked(const FIntPoint& Cell) const
{
    //二维坐标转一维索引的标准公式
    //FlatIndex = Y * Width + X
    const int64 FlatIndex =
        static_cast<int64>(Cell.Y) *
        static_cast<int64>(Config.Width) +
        static_cast<int64>(Cell.X);

    return static_cast<int32>(FlatIndex);
}

bool FNiumaSpatialContainer::IsInitialized() const
{
    return bInitialized;
}

const FNiumaSpatialContainerConfig& FNiumaSpatialContainer::GetConfig() const
{
    return Config;
}

const FNiumaSpatialContainerState& FNiumaSpatialContainer::GetState() const
{
    return State;
}


ENiumaWarehouseOperationResult FNiumaSpatialContainer::CanPlace(
    const FNiumaSpatialItemPlacement& Placement,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError) const
{
    /*
    * 容器是否初始化
      → Count 是否合法
      → Item 是否合法
      → Placement 是否合法
      → Resolver 能否找到定义
      → 解析结果是否有效
      → 容器是否接收该类型
      → 当前方向是否被允许
      → Footprint 是否越界
      → Footprint 是否发生碰撞
      → Success
    */
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    // Count 单独返回 InvalidCount，
    // 而不是笼统归类为 InvalidItem。
    if (Placement.Item.Count <= 0)
    {
        SetSpatialContainerError(OutError,TEXT("物品数量必须大于 0"));

        return ENiumaWarehouseOperationResult::InvalidCount;
    }

    FString ItemError;

    if (!Placement.Item.IsValid(&ItemError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(TEXT("物品实例无效：%s"), *ItemError);
        }

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    FString PlacementError;

    if (!Placement.IsValid(&PlacementError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(TEXT("放置记录无效：%s"), *PlacementError);
        }

        return ENiumaWarehouseOperationResult::InvalidPlacement;
    }

    FNiumaResolvedItemSpatialData ResolvedData;
    FString ResolveError;

    if (!Resolver.TryResolve(
        Placement.Item.ItemDefinitionId,
        Placement.Orientation,
        ResolvedData,
        &ResolveError))
    {
        if (OutError != nullptr)
        {
            *OutError = ResolveError.IsEmpty()
                ? TEXT("找不到物品空间定义")
                : FString::Printf(TEXT("物品空间定义解析失败：%s"), *ResolveError);
        }

        return
            ENiumaWarehouseOperationResult::
            MissingItemDefinition;
    }

    FString ResolvedDataError;

    if (!ResolvedData.IsStructurallyValid(&ResolvedDataError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(TEXT("物品空间定义无效：%s"), *ResolvedDataError);
        }

        return ENiumaWarehouseOperationResult::InvalidItemDefinition;
    }

    if (!Config.AcceptedItemTypes.Contains(ResolvedData.ItemType))
    {
        SetSpatialContainerError(OutError,TEXT("当前容器不接收该物品类型"));

        return ENiumaWarehouseOperationResult::UnsupportedItemType;
    }

    if (ResolvedData.RotationPolicy ==
        ENiumaItemRotationPolicy::Fixed &&
        Placement.Orientation !=
        ENiumaItemOrientation::Degree0)
    {
        SetSpatialContainerError(OutError,TEXT("该物品不允许旋转"));

        return ENiumaWarehouseOperationResult::RotationNotAllowed;
    }

    for (const FIntPoint& LocalCell : ResolvedData.Footprint.Cells)
    {
        // 使用 int64 做加法，避免巨大坐标在边界检查前溢出。
        const int64 WorldX =
            static_cast<int64>(Placement.Origin.X) +
            static_cast<int64>(LocalCell.X);

        const int64 WorldY =
            static_cast<int64>(Placement.Origin.Y) +
            static_cast<int64>(LocalCell.Y);

        if (WorldX < 0 ||
            WorldY < 0 ||
            WorldX >= static_cast<int64>(Config.Width) ||
            WorldY >= static_cast<int64>(Config.Height))
        {
            if (OutError != nullptr)
            {
                *OutError = FString::Printf(
                    TEXT("Footprint 逻辑格 (%lld, %lld)超出容器范围"),
                    WorldX,
                    WorldY);
            }

            return
                ENiumaWarehouseOperationResult::
                OutOfBounds;
        }

        const FIntPoint WorldCell(
            static_cast<int32>(WorldX),
            static_cast<int32>(WorldY));

        const int32 FlatIndex =
            ToFlatIndexUnchecked(WorldCell);

        if (!OccupancyCache.IsValidIndex(FlatIndex))
        {
            SetSpatialContainerError(
                OutError,
                TEXT("空间容器占用缓存与配置不一致"));

            return
                ENiumaWarehouseOperationResult::
                InternalError;
        }

        const int32 OccupyingPlacementIndex =
            OccupancyCache[FlatIndex];

        if (OccupyingPlacementIndex != INDEX_NONE)
        {
            // 缓存中的非空值必须是合法 Placement 下标。
            if (!State.Placements.IsValidIndex(OccupyingPlacementIndex))
            {
                SetSpatialContainerError(
                    OutError,
                    TEXT("占用缓存包含无效的Placement 下标"));

                return ENiumaWarehouseOperationResult::InternalError;
            }

            if (OutError != nullptr)
            {
                *OutError = FString::Printf(
                    TEXT("目标逻辑格 (%d, %d)已被占用"),
                    WorldCell.X,
                    WorldCell.Y);
            }

            return ENiumaWarehouseOperationResult::Occupied;
        }
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return ENiumaWarehouseOperationResult::Success;
}