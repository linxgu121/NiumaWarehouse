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

bool FNiumaSpatialContainer::TryInitializeFromState(
    const FNiumaSpatialContainerConfig& InConfig,
    const FNiumaSpatialContainerState& InState,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError)
{
    if (bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器已经初始化，不能重复初始化"));

        return false;
    }

    /*
     * 所有恢复操作都在候选容器中完成。
     * 正式容器在最后提交前保持未初始化状态。
     */
    FNiumaSpatialContainer CandidateContainer;

    FString InitializeError;

    if (!CandidateContainer.TryInitializeEmpty(
        InConfig,
        &InitializeError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("恢复空间容器配置失败：%s"),
                *InitializeError);
        }

        return false;
    }

    FString StateError;

    if (!InState.IsStructurallyValid(&StateError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("待恢复的容器 State 无效：%s"),
                *StateError);
        }

        return false;
    }

    /*
     * 按原顺序重新放置每条记录。
     * TryPlace 会负责：
     * - 定义解析
     * - 类型与旋转许可
     * - 越界和碰撞
     * - 重复实例检查
     * - OccupancyCache 写入
     */
    for (int32 PlacementIndex = 0;
        PlacementIndex < InState.Placements.Num();
        ++PlacementIndex)
    {
        const FNiumaSpatialItemPlacement& Placement =
            InState.Placements[PlacementIndex];

        FString PlacementError;

        const ENiumaWarehouseOperationResult PlacementResult =CandidateContainer.TryPlace(
                Placement,
                Resolver,
                &PlacementError);

        if (PlacementResult != ENiumaWarehouseOperationResult::Success)
        {
            if (OutError != nullptr)
            {
                *OutError = FString::Printf(
                    TEXT(
                        "重建第 %d 条 Placement 失败（结果值：%d）：%s"),
                    PlacementIndex,
                    static_cast<int32>(PlacementResult),
                    *PlacementError);
            }

            return false;
        }
    }

    /*
     * TryPlace 在候选重建过程中临时增加了 Revision。
     * 重建不是新的业务修改，最终必须恢复存档中的版本号。
     */
    CandidateContainer.State.Revision = InState.Revision;

    FString RebuiltStateError;

    if (!CandidateContainer.State.IsStructurallyValid(&RebuiltStateError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("重建后的容器 State 无效：%s"),
                *RebuiltStateError);
        }

        return false;
    }

    /*
     * 所有 Placement 和缓存都成功后，
     * 才一次性提交正式成员。
     */
    Config = MoveTemp(CandidateContainer.Config);

    State = MoveTemp(CandidateContainer.State);

    OccupancyCache = MoveTemp(CandidateContainer.OccupancyCache);

    bInitialized = CandidateContainer.bInitialized;

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

//事务边界说明
//EvaluatePlacement 成功
//↓
//复制 CandidateState
//复制 CandidateOccupancy
//↓
//向候选 State 添加 Placement
//向候选 Occupancy 写入全部 Footprint
//Revision + 1
//验证候选 State
//↓
//一次性替换正式 State 与 Occupancy


ENiumaWarehouseOperationResult FNiumaSpatialContainer::EvaluatePlacement(
    const FNiumaSpatialItemPlacement& Placement,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    int32 IgnoredPlacementIndex,
    FNiumaResolvedItemSpatialData* OutResolvedData,
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

    if (IgnoredPlacementIndex != INDEX_NONE && !State.Placements.IsValidIndex(IgnoredPlacementIndex))
    {
        SetSpatialContainerError(OutError,TEXT("待忽略的 Placement 下标无效"));

        return ENiumaWarehouseOperationResult::InternalError;
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

    const ENiumaWarehouseOperationResult ResolvedResult = EvaluateResolvedPlacement(
            Placement,
            ResolvedData,
            IgnoredPlacementIndex,
            OutError);

    if (ResolvedResult != ENiumaWarehouseOperationResult::Success)
    {
        return ResolvedResult;
    }

    // 只有全部规则通过后，才提交可选输出。
    if (OutResolvedData != nullptr)
    {
        *OutResolvedData = MoveTemp(ResolvedData);
    }

    return ENiumaWarehouseOperationResult::Success;
}

ENiumaWarehouseOperationResult
FNiumaSpatialContainer::EvaluateResolvedPlacement(
    const FNiumaSpatialItemPlacement& Placement,
    const FNiumaResolvedItemSpatialData& ResolvedData,
    int32 IgnoredPlacementIndex,
    FString* OutError) const
{
    FString ResolvedDataError;

    if (!ResolvedData.IsStructurallyValid(
        &ResolvedDataError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("物品空间定义无效：%s"),
                *ResolvedDataError);
        }

        return
            ENiumaWarehouseOperationResult::
            InvalidItemDefinition;
    }

    if (!Config.AcceptedItemTypes.Contains(
        ResolvedData.ItemType))
    {
        SetSpatialContainerError(
            OutError,
            TEXT("当前容器不接收该物品类型"));

        return
            ENiumaWarehouseOperationResult::
            UnsupportedItemType;
    }

    if (ResolvedData.RotationPolicy ==
            ENiumaItemRotationPolicy::Fixed &&
        Placement.Orientation !=
            ENiumaItemOrientation::Degree0)
    {
        SetSpatialContainerError(
            OutError,
            TEXT("该物品不允许旋转"));

        return
            ENiumaWarehouseOperationResult::
            RotationNotAllowed;
    }

    for (const FIntPoint& LocalCell :
         ResolvedData.Footprint.Cells)
    {
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
                    TEXT(
                        "Footprint 逻辑格 (%lld, %lld) "
                        "超出容器范围"),
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

        if (OccupyingPlacementIndex == INDEX_NONE)
        {
            continue;
        }

        if (!State.Placements.IsValidIndex(
            OccupyingPlacementIndex))
        {
            SetSpatialContainerError(
                OutError,
                TEXT(
                    "占用缓存包含无效的 "
                    "Placement 下标"));

            return
                ENiumaWarehouseOperationResult::
                InternalError;
        }

        if (OccupyingPlacementIndex ==
            IgnoredPlacementIndex)
        {
            continue;
        }

        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT(
                    "目标逻辑格 (%d, %d) "
                    "已被占用"),
                WorldCell.X,
                WorldCell.Y);
        }

        return
            ENiumaWarehouseOperationResult::
            Occupied;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return ENiumaWarehouseOperationResult::Success;
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::CanPlace(
    const FNiumaSpatialItemPlacement& Placement,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError) const
{
    return EvaluatePlacement(Placement,Resolver,INDEX_NONE,nullptr,OutError);
}

int32 FNiumaSpatialContainer::FindPlacementIndexByInstanceId(
    const FGuid& InstanceId) const
{
    for (int32 Index = 0;
        Index < State.Placements.Num();
        ++Index)
    {
        if (State.Placements[Index].Item.InstanceId ==
            InstanceId)
        {
            return Index;
        }
    }

    return INDEX_NONE;
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::FindFirstValidPlacement(
    const FNiumaItemInstance& Item,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FNiumaSpatialItemPlacement& OutPlacement,
    FString* OutError) const
{
    //1. 前置校验（初始化、物品合法、不重复）
    //2. 预定义搜索方向：[0°, 90°, 180°, 270°]
    //3. 对每个格子(X, Y)：
    //    对每个方向：
    //  - 首次遇到该方向 → 调用 Resolver 解析 Footprint（缓存）
    //  - 如果 0° 解析后发现 RotationPolicy == Fixed → 后续方向不再尝试
    //  - 用已解析的数据做碰撞 / 边界评估
    //  - 成功 → 返回这个位置和方向
    //  - 被占 / 越界 → 继续试下一个
    //  - 其他错误（如定义无效）→ 直接返回失败
    //4. 全部扫完都没找到 → NoValidPlacement
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (Item.Count <= 0)
    {
        SetSpatialContainerError(OutError,TEXT("物品数量必须大于 0"));

        return ENiumaWarehouseOperationResult::InvalidCount;
    }

    FString ItemError;

    if (!Item.IsValid(&ItemError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(TEXT("物品实例无效：%s"),*ItemError);
        }

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    if (FindPlacementIndexByInstanceId(Item.InstanceId) !=
        INDEX_NONE)
    {
        SetSpatialContainerError(OutError,TEXT("该物品实例已经存在于容器中"));

        return ENiumaWarehouseOperationResult::ItemAlreadyExists;
    }

    static constexpr ENiumaItemOrientation SearchOrientations[] =
    {
        ENiumaItemOrientation::Degree0,
        ENiumaItemOrientation::Degree90,
        ENiumaItemOrientation::Degree180,
        ENiumaItemOrientation::Degree270
    };

    constexpr int32 MaxOrientationCount = UE_ARRAY_COUNT(SearchOrientations);

    FNiumaResolvedItemSpatialData ResolvedDataCache[MaxOrientationCount];

    bool bHasResolvedData[MaxOrientationCount] = {};

    int32 ActiveOrientationCount = MaxOrientationCount;

    FNiumaSpatialItemPlacement CandidatePlacement;
    CandidatePlacement.Item = Item;

    for (int32 Y = 0; Y < Config.Height; ++Y)
    {
        for (int32 X = 0; X < Config.Width; ++X)
        {
            CandidatePlacement.Origin = FIntPoint(X, Y);

            for (int32 OrientationIndex = 0;
                OrientationIndex < ActiveOrientationCount;
                ++OrientationIndex)
            {
                const ENiumaItemOrientation Orientation = SearchOrientations[OrientationIndex];

                CandidatePlacement.Orientation = Orientation;

                if (!bHasResolvedData[OrientationIndex])
                {
                    FString ResolveError;

                    if (!Resolver.TryResolve(
                        Item.ItemDefinitionId,
                        Orientation,
                        ResolvedDataCache[OrientationIndex],
                        &ResolveError))
                    {
                        if (OutError != nullptr)
                        {
                            *OutError = ResolveError.IsEmpty()
                                ? TEXT("找不到物品空间定义")
                                : FString::Printf(
                                    TEXT("物品空间定义解析失败：%s"),
                                    *ResolveError);
                        }

                        return ENiumaWarehouseOperationResult::MissingItemDefinition;
                    }

                    bHasResolvedData[OrientationIndex] = true;

                    // 0° 数据确定物品是否允许继续尝试其他方向。
                    if (OrientationIndex == 0 &&
                        ResolvedDataCache[0].RotationPolicy ==
                        ENiumaItemRotationPolicy::Fixed)
                    {
                        ActiveOrientationCount = 1;
                    }
                }

                const ENiumaWarehouseOperationResult Result =
                    EvaluateResolvedPlacement(
                        CandidatePlacement,
                        ResolvedDataCache[OrientationIndex],
                        INDEX_NONE,
                        OutError);

                if (Result ==
                    ENiumaWarehouseOperationResult::Success)
                {
                    // 只有成功时才修改正式输出参数。
                    OutPlacement = CandidatePlacement;

                    if (OutError != nullptr)
                    {
                        OutError->Reset();
                    }

                    return
                        ENiumaWarehouseOperationResult::Success;
                }

                // 这两种结果只代表当前位置不可用，
                // 应继续搜索后面的候选位置。
                if (Result != ENiumaWarehouseOperationResult::Occupied &&
                    Result != ENiumaWarehouseOperationResult::OutOfBounds)
                {
                    return Result;
                }
            }
        }
    }

    SetSpatialContainerError(OutError,TEXT("当前容器没有可用放置位置"));

    return ENiumaWarehouseOperationResult::NoValidPlacement;
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::TryFindPlacement(
    const FGuid& InstanceId,
    FNiumaSpatialItemPlacement& OutPlacement,
    FString* OutError) const
{
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (!InstanceId.IsValid())
    {
        SetSpatialContainerError(OutError,TEXT("待查询的 InstanceId 无效"));

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    const int32 PlacementIndex = FindPlacementIndexByInstanceId(InstanceId);

    if (PlacementIndex == INDEX_NONE)
    {
        SetSpatialContainerError(OutError,TEXT("容器中找不到指定物品实例"));

        return ENiumaWarehouseOperationResult::ItemNotFound;
    }

    if (!State.Placements.IsValidIndex(PlacementIndex))
    {
        SetSpatialContainerError(OutError,TEXT("物品查询下标与容器状态不一致"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    const FNiumaSpatialItemPlacement& FoundPlacement =
        State.Placements[PlacementIndex];

    if (!FoundPlacement.IsValid(nullptr))
    {
        SetSpatialContainerError(OutError,TEXT("容器中的放置记录结构无效"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    // 所有检查通过后才提交输出，保证失败原子性。
    OutPlacement = FoundPlacement;

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return ENiumaWarehouseOperationResult::Success;
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::TryPlace(
    const FNiumaSpatialItemPlacement& Placement,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError)
{
    /*
     * 只有结构合法的 Placement 才检查重复实例。
     *
     * 非法输入继续交给 EvaluatePlacement，
     * 从而保持 InvalidCount、InvalidItem、
     * InvalidPlacement 的原有错误优先级。
     */
    if (bInitialized && Placement.IsValid(nullptr) &&
        FindPlacementIndexByInstanceId(Placement.Item.InstanceId) != INDEX_NONE)
    {
        SetSpatialContainerError(OutError,TEXT("该物品实例已经存在于容器中"));

        return ENiumaWarehouseOperationResult::ItemAlreadyExists;
    }

    FNiumaResolvedItemSpatialData ResolvedData;

    const ENiumaWarehouseOperationResult EvaluationResult =
        EvaluatePlacement(Placement,Resolver,INDEX_NONE,&ResolvedData,OutError);

    if (EvaluationResult != ENiumaWarehouseOperationResult::Success)
    {
        return EvaluationResult;
    }

    if (State.Revision < 0 || State.Revision == MAX_int64)
    {
        SetSpatialContainerError(OutError,TEXT("容器 Revision 无法继续增加"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    /*
     * 从这里开始只修改候选副本。
     * 任何 return 都不会影响正式容器。
     */
    FNiumaSpatialContainerState CandidateState = State;

    TArray<int32> CandidateOccupancy = OccupancyCache;

    const int32 NewPlacementIndex = CandidateState.Placements.Add(Placement);

    for (const FIntPoint& LocalCell : ResolvedData.Footprint.Cells)
    {
        const int64 WorldX =
            static_cast<int64>(Placement.Origin.X) +
            static_cast<int64>(LocalCell.X);

        const int64 WorldY =
            static_cast<int64>(Placement.Origin.Y) +
            static_cast<int64>(LocalCell.Y);

        /*
         * EvaluatePlacement 已经验证过边界，
         * 这里使用同一份 ResolvedData 重建坐标。
         */
        const FIntPoint WorldCell(
            static_cast<int32>(WorldX),
            static_cast<int32>(WorldY));

        const int32 FlatIndex = ToFlatIndexUnchecked(WorldCell);

        if (!CandidateOccupancy.IsValidIndex(FlatIndex))
        {
            SetSpatialContainerError(OutError,TEXT("候选占用缓存与容器配置不一致"));

            return ENiumaWarehouseOperationResult::InternalError;
        }

        /*
         * EvaluatePlacement 已确认这些格为空。
         * 再检查一次可以保护提交阶段的不变量。
         */
        if (CandidateOccupancy[FlatIndex] != INDEX_NONE)
        {
            SetSpatialContainerError(OutError,TEXT("候选占用缓存出现意外碰撞"));

            return ENiumaWarehouseOperationResult::InternalError;
        }

        CandidateOccupancy[FlatIndex] = NewPlacementIndex;
    }

    ++CandidateState.Revision;

    FString CandidateStateError;

    if (!CandidateState.IsStructurallyValid(&CandidateStateError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(TEXT("候选容器状态无效：%s"), *CandidateStateError);
        }

        return ENiumaWarehouseOperationResult::InternalError;
    }

    /*
     * 全部空间写入与状态验证成功后，
     * 才一次性提交两个正式成员。
     */
    State = MoveTemp(CandidateState);
    OccupancyCache = MoveTemp(CandidateOccupancy);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return ENiumaWarehouseOperationResult::Success;
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::TryRemove(
    const FGuid& InstanceId,
    FString* OutError)
{
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (!InstanceId.IsValid())
    {
        SetSpatialContainerError(OutError,TEXT("待移除的 InstanceId 无效"));

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    const int32 RemovedPlacementIndex = FindPlacementIndexByInstanceId(InstanceId);

    if (RemovedPlacementIndex == INDEX_NONE)
    {
        SetSpatialContainerError(OutError,TEXT("容器中找不到指定物品实例"));

        return ENiumaWarehouseOperationResult::ItemNotFound;
    }

    if (State.Revision < 0 || State.Revision == MAX_int64)
    {
        SetSpatialContainerError(OutError,TEXT("容器 Revision 无法继续增加"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    FNiumaSpatialContainerState CandidateState = State;

    TArray<int32> CandidateOccupancy = OccupancyCache;

    /*
     * Occupancy 当前保存的是旧 Placements 下标。
     *
     * 删除下标 N 后：
     * - 等于 N：属于被删除物品，释放为空格；
     * - 大于 N：TArray 元素前移，下标减 1；
     * - 小于 N：位置不变。
     */
    for (int32& OccupyingPlacementIndex : CandidateOccupancy)
    {
        if (OccupyingPlacementIndex == INDEX_NONE)
        {
            continue;
        }

        if (!State.Placements.IsValidIndex(OccupyingPlacementIndex))
        {
            SetSpatialContainerError(OutError,TEXT("占用缓存包含无效的Placement 下标"));

            return ENiumaWarehouseOperationResult::InternalError;
        }

        if (OccupyingPlacementIndex == RemovedPlacementIndex)
        {
            OccupyingPlacementIndex = INDEX_NONE;
        }
        else if (OccupyingPlacementIndex > RemovedPlacementIndex)
        {
            --OccupyingPlacementIndex;
        }
    }

    /*
     * RemoveAt 保持剩余元素顺序，
     * 后面的 Placement 会整体向前移动一位。
     */
    CandidateState.Placements.RemoveAt(RemovedPlacementIndex);

    ++CandidateState.Revision;

    FString CandidateStateError;

    if (!CandidateState.IsStructurallyValid(&CandidateStateError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("候选容器状态无效：%s"),
                *CandidateStateError);
        }

        return ENiumaWarehouseOperationResult::InternalError;
    }

    State = MoveTemp(CandidateState);
    OccupancyCache = MoveTemp(CandidateOccupancy);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return ENiumaWarehouseOperationResult::Success;
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::CanRelocate(
    const FGuid& InstanceId,
    FIntPoint NewOrigin,
    ENiumaItemOrientation NewOrientation,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError) const
{
    if (!bInitialized)
    {
        SetSpatialContainerError(
            OutError,
            TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (!InstanceId.IsValid())
    {
        SetSpatialContainerError(
            OutError,
            TEXT("待预览重定位的 InstanceId 无效"));

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    const int32 PlacementIndex = FindPlacementIndexByInstanceId(InstanceId);

    if (PlacementIndex == INDEX_NONE)
    {
        SetSpatialContainerError(
            OutError,
            TEXT("容器中找不到待预览重定位的物品实例"));

        return ENiumaWarehouseOperationResult::ItemNotFound;
    }

    if (!State.Placements.IsValidIndex(PlacementIndex))
    {
        SetSpatialContainerError(
            OutError,
            TEXT("待预览重定位的 Placement 下标无效"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    const FNiumaSpatialItemPlacement& ExistingPlacement =
        State.Placements[PlacementIndex];

    // 预览当前位置属于成功的无操作。
    if (ExistingPlacement.Origin == NewOrigin &&
        ExistingPlacement.Orientation == NewOrientation)
    {
        if (OutError != nullptr)
        {
            OutError->Reset();
        }

        return ENiumaWarehouseOperationResult::Success;
    }

    FNiumaSpatialItemPlacement CandidatePlacement = ExistingPlacement;

    CandidatePlacement.Origin = NewOrigin;
    CandidatePlacement.Orientation = NewOrientation;

    return EvaluatePlacement(
        CandidatePlacement,
        Resolver,
        PlacementIndex,
        nullptr,
        OutError);
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::TryRelocate(
    const FGuid& InstanceId,
    FIntPoint NewOrigin,
    ENiumaItemOrientation NewOrientation,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError)
{
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (!InstanceId.IsValid())
    {
        SetSpatialContainerError(OutError,TEXT("待重定位的 InstanceId 无效"));

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    const int32 PlacementIndex = FindPlacementIndexByInstanceId(InstanceId);

    if (PlacementIndex == INDEX_NONE)
    {
        SetSpatialContainerError(
            OutError,
            TEXT("容器中找不到待重定位的物品实例"));

        return ENiumaWarehouseOperationResult::ItemNotFound;
    }

    if (!State.Placements.IsValidIndex(PlacementIndex))
    {
        SetSpatialContainerError(
            OutError,
            TEXT("待重定位的 Placement 下标无效"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    const FNiumaSpatialItemPlacement& ExistingPlacement =
        State.Placements[PlacementIndex];

    /*
     * 位置与方向均未变化时属于成功无操作。
     * 不解析物品定义，也不增加 Revision。
     */
    if (ExistingPlacement.Origin == NewOrigin &&
        ExistingPlacement.Orientation == NewOrientation)
    {
        if (OutError != nullptr)
        {
            OutError->Reset();
        }

        return ENiumaWarehouseOperationResult::Success;
    }

    FNiumaSpatialItemPlacement CandidatePlacement = ExistingPlacement;

    CandidatePlacement.Origin = NewOrigin;
    CandidatePlacement.Orientation = NewOrientation;

    return TryReplacePlacementAt(
        PlacementIndex,
        CandidatePlacement,
        Resolver,
        OutError);
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::TryMove(
    const FGuid& InstanceId,
    FIntPoint NewOrigin,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError)
{
    //移动事务过程
    //找到原 Placement
    //→ 构造只改变 Origin 的候选 Placement
    //→ EvaluatePlacement 忽略自己的旧下标
    //→ 复制 State 与 Occupancy
    //→ 候选缓存释放旧占用
    //→ 候选缓存写入新占用
    //→ 替换候选 Placement
    //→ Revision + 1
    //→ 一次性提交
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (!InstanceId.IsValid())
    {
        SetSpatialContainerError(OutError,TEXT("待移动的 InstanceId 无效"));

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    const int32 PlacementIndex = FindPlacementIndexByInstanceId(InstanceId);

    if (PlacementIndex == INDEX_NONE)
    {
        SetSpatialContainerError(OutError,TEXT("容器中找不到待移动的物品实例"));

        return ENiumaWarehouseOperationResult::ItemNotFound;
    }

    const FNiumaSpatialItemPlacement& ExistingPlacement = State.Placements[PlacementIndex];

    /*
     * 原点没有变化时属于成功的无操作，
     * 不调用 Resolver，也不增加 Revision。
     */
    if (ExistingPlacement.Origin == NewOrigin)
    {
        if (OutError != nullptr)
        {
            OutError->Reset();
        }

        return ENiumaWarehouseOperationResult::Success;
    }

    FNiumaSpatialItemPlacement CandidatePlacement = ExistingPlacement;

    CandidatePlacement.Origin = NewOrigin;

    return TryReplacePlacementAt(
        PlacementIndex,
        CandidatePlacement,
        Resolver,
        OutError);
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::TryReplacePlacementAt(
    int32 PlacementIndex,
    const FNiumaSpatialItemPlacement& CandidatePlacement,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError)
{
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (!State.Placements.IsValidIndex(PlacementIndex))
    {
        SetSpatialContainerError(OutError,TEXT("待替换的 Placement 下标无效"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    const FNiumaItemInstance& ExistingItem = State.Placements[PlacementIndex].Item;

    const FNiumaItemInstance& CandidateItem = CandidatePlacement.Item;

    /*
     * 移动与旋转只能改变空间数据，
     * 不能借替换操作修改物品实例。
     */
    const bool bSameItem =
        ExistingItem.InstanceId ==
        CandidateItem.InstanceId &&
        ExistingItem.ItemDefinitionId ==
        CandidateItem.ItemDefinitionId &&
        ExistingItem.Count ==
        CandidateItem.Count;

    if (!bSameItem)
    {
        SetSpatialContainerError(OutError,TEXT("候选 Placement 修改了物品实例数据"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    FNiumaResolvedItemSpatialData ResolvedData;

    const ENiumaWarehouseOperationResult EvaluationResult =
        EvaluatePlacement(
            CandidatePlacement,
            Resolver,
            PlacementIndex,
            &ResolvedData,
            OutError);

    if (EvaluationResult !=
        ENiumaWarehouseOperationResult::Success)
    {
        return EvaluationResult;
    }

    if (State.Revision < 0 || State.Revision == MAX_int64)
    {
        SetSpatialContainerError(OutError,TEXT("容器 Revision 无法继续增加"));

        return ENiumaWarehouseOperationResult::InternalError;
    }

    FNiumaSpatialContainerState CandidateState = State;

    TArray<int32> CandidateOccupancy = OccupancyCache;

    /*
     * 先在候选缓存中释放原 Placement 的全部旧占用。
     * 其他 Placement 的下标和占用保持不变。
     */
    for (int32& OccupyingPlacementIndex : CandidateOccupancy)
    {
        if (OccupyingPlacementIndex == INDEX_NONE)
        {
            continue;
        }

        if (!State.Placements.IsValidIndex(OccupyingPlacementIndex))
        {
            SetSpatialContainerError(OutError,TEXT("占用缓存包含无效的Placement 下标"));

            return ENiumaWarehouseOperationResult::InternalError;
        }

        if (OccupyingPlacementIndex == PlacementIndex)
        {
            OccupyingPlacementIndex = INDEX_NONE;
        }
    }

    /*
     * 使用本次 EvaluatePlacement 输出的同一份
     * ResolvedData 写入候选新占用。
     */
    for (const FIntPoint& LocalCell :
        ResolvedData.Footprint.Cells)
    {
        const int64 WorldX =
            static_cast<int64>(
                CandidatePlacement.Origin.X) +
            static_cast<int64>(LocalCell.X);

        const int64 WorldY =
            static_cast<int64>(
                CandidatePlacement.Origin.Y) +
            static_cast<int64>(LocalCell.Y);

        const FIntPoint WorldCell(
            static_cast<int32>(WorldX),
            static_cast<int32>(WorldY));

        const int32 FlatIndex = ToFlatIndexUnchecked(WorldCell);

        if (!CandidateOccupancy.IsValidIndex(FlatIndex))
        {
            SetSpatialContainerError(OutError,TEXT("候选占用缓存与容器配置不一致"));

            return ENiumaWarehouseOperationResult::InternalError;
        }

        /*
         * 自身旧占用已经释放；
         * 如果这里仍非空，只可能是内部状态异常。
         */
        if (CandidateOccupancy[FlatIndex] !=INDEX_NONE)
        {
            SetSpatialContainerError(OutError,TEXT("候选占用缓存出现意外碰撞"));

            return ENiumaWarehouseOperationResult::InternalError;
        }

        CandidateOccupancy[FlatIndex] = PlacementIndex;
    }

    CandidateState.Placements[PlacementIndex] = CandidatePlacement;

    ++CandidateState.Revision;

    FString CandidateStateError;

    if (!CandidateState.IsStructurallyValid(&CandidateStateError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("候选容器状态无效：%s"),
                *CandidateStateError);
        }

        return ENiumaWarehouseOperationResult::InternalError;
    }

    State = MoveTemp(CandidateState);
    OccupancyCache = MoveTemp(CandidateOccupancy);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return ENiumaWarehouseOperationResult::Success;
}

ENiumaWarehouseOperationResult FNiumaSpatialContainer::TryRotate(
    const FGuid& InstanceId,
    ENiumaItemOrientation NewOrientation,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FString* OutError)
{
    //业务逻辑
    //TryRotate
    //→ 找到 Placement
    //→ 复制 Placement
    //→ 只修改 Orientation
    //→ TryReplacePlacementAt
    //→ Resolver 返回新方向 Footprint
    //→ 忽略自身旧占用进行碰撞检查
    //→ 候选缓存释放旧形状
    //→ 候选缓存写入旋转后形状
    //→ Revision + 1
    //→ 原子提交
    if (!bInitialized)
    {
        SetSpatialContainerError(OutError,TEXT("空间容器尚未初始化"));

        return ENiumaWarehouseOperationResult::NotInitialized;
    }

    if (!InstanceId.IsValid())
    {
        SetSpatialContainerError(OutError,TEXT("待旋转的 InstanceId 无效"));

        return ENiumaWarehouseOperationResult::InvalidItem;
    }

    const int32 PlacementIndex = FindPlacementIndexByInstanceId(InstanceId);

    if (PlacementIndex == INDEX_NONE)
    {
        SetSpatialContainerError(OutError, TEXT("容器中找不到待旋转的物品实例"));

        return ENiumaWarehouseOperationResult::ItemNotFound;
    }

    const FNiumaSpatialItemPlacement& ExistingPlacement = State.Placements[PlacementIndex];

    /*
     * 方向没有变化时属于成功的无操作。
     * 不调用 Resolver，也不增加 Revision。
     */
    if (ExistingPlacement.Orientation == NewOrientation)
    {
        if (OutError != nullptr)
        {
            OutError->Reset();
        }

        return ENiumaWarehouseOperationResult::Success;
    }

    FNiumaSpatialItemPlacement CandidatePlacement = ExistingPlacement;

    CandidatePlacement.Orientation = NewOrientation;

    return TryReplacePlacementAt(
        PlacementIndex,
        CandidatePlacement,
        Resolver,
        OutError);
}
