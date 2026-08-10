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