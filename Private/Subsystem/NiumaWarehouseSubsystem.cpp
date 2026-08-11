#include "NiumaWarehouse/Subsystem/NiumaWarehouseSubsystem.h"

#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"

#include "NiumaWarehouse/Definitions/NiumaWarehouseDefinition.h"
#include "NiumaWarehouse/Settings/NiumaWarehouseSettings.h"
#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"

DEFINE_LOG_CATEGORY_STATIC(LogNiumaWarehouseSubsystem,Log,All);

namespace
{
    void SetInitializationError(
        FString* OutError,
        const FString& Message)
    {
        if (OutError != nullptr)
        {
            *OutError = Message;
        }
    }
}

void UNiumaWarehouseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializationError.Reset();

    if (!TryInitializeDefaultWarehouse(&InitializationError))
    {
        UE_LOG(
            LogNiumaWarehouseSubsystem,
            Error,
            TEXT("默认仓库初始化失败：%s"),
            *InitializationError);

        return;
    }

    const FNiumaSpatialContainerConfig& Config = Warehouse.GetConfig();

    UE_LOG(
        LogNiumaWarehouseSubsystem,
        Log,
        TEXT("默认仓库初始化成功：%d x %d"),
        Config.Width,
        Config.Height);
}

void UNiumaWarehouseSubsystem::Deinitialize()
{
    OnWarehouseChanged.Clear();

    Super::Deinitialize();
}

bool UNiumaWarehouseSubsystem::TryInitializeDefaultWarehouse(FString* OutError)
{
    const UNiumaWarehouseSettings* Settings = GetDefault<UNiumaWarehouseSettings>();

    if (Settings == nullptr)
    {
        SetInitializationError(
            OutError,
            TEXT("无法读取仓库项目设置"));

        return false;
    }

    const FSoftObjectPath DefinitionPath =
        Settings
        ->DefaultWarehouseDefinition
        .ToSoftObjectPath();

    if (!DefinitionPath.IsValid())
    {
        SetInitializationError(
            OutError,
            TEXT("没有配置默认仓库定义"));

        return false;
    }

    const UNiumaWarehouseDefinition* Definition =
        Settings
        ->DefaultWarehouseDefinition
        .LoadSynchronous();

    if (Definition == nullptr)
    {
        SetInitializationError(
            OutError,
            FString::Printf(
                TEXT("无法加载默认仓库定义：%s"),
                *DefinitionPath.ToString()));

        return false;
    }

    FNiumaSpatialContainerConfig ContainerConfig;
    FString ConfigError;

    if (!FNiumaSpatialContainerConfig::TryCreate(
        ENiumaItemContainerType::Warehouse,
        Definition->Width,
        Definition->Height,
        Definition->AcceptedItemTypes,
        ContainerConfig,
        &ConfigError))
    {
        SetInitializationError(
            OutError,
            FString::Printf(
                TEXT("默认仓库配置无效：%s"),
                *ConfigError));

        return false;
    }

    FString ContainerError;

    if (!Warehouse.TryInitializeEmpty(ContainerConfig,&ContainerError))
    {
        SetInitializationError(
            OutError,
            FString::Printf(
                TEXT("空仓库初始化失败：%s"),
                *ContainerError));

        return false;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}

FNiumaWarehouseOperationResponse UNiumaWarehouseSubsystem::TryReceiveItem(
    const FNiumaItemInstance& Item)
{
    FNiumaSpatialItemPlacement Placement;
    FString Error;

    const ENiumaWarehouseOperationResult
        FindResult = Warehouse.FindFirstValidPlacement(
            Item,
            ItemDefinitionResolver,
            Placement,
            &Error);

    if (FindResult != ENiumaWarehouseOperationResult::Success)
    {
        return FNiumaWarehouseOperationResponse::MakeFailure(
                FindResult,
                Error);
    }

    /*
    * 提交前先验证能否构造成功响应，
    * 避免仓库已经修改后才发现响应数据异常。
    */
    const FNiumaWarehouseOperationResponse SuccessResponse =
        FNiumaWarehouseOperationResponse::MakeSuccess(Placement);

    if (!SuccessResponse.IsSuccess())
    {
        return SuccessResponse;
    }

    Error.Reset();

    const ENiumaWarehouseOperationResult
        PlaceResult = Warehouse.TryPlace(
            Placement,
            ItemDefinitionResolver,
            &Error);

    if (PlaceResult != ENiumaWarehouseOperationResult::Success)
    {
        return FNiumaWarehouseOperationResponse::MakeFailure(
                PlaceResult,
                Error);
    }

    /*
     * 此时 Placement、Occupancy 和 Revision
     * 已经全部成功提交。
     */
    OnWarehouseChanged.Broadcast(Warehouse.GetState().Revision,SuccessResponse);

    return SuccessResponse;

}



bool UNiumaWarehouseSubsystem::IsWarehouseInitialized() const
{
    return Warehouse.IsInitialized();
}

FString UNiumaWarehouseSubsystem::GetInitializationError() const
{
    return InitializationError;
}

const FNiumaSpatialContainerConfig& UNiumaWarehouseSubsystem::GetWarehouseConfig() const
{
    return Warehouse.GetConfig();
}

const FNiumaSpatialContainerState& UNiumaWarehouseSubsystem::GetWarehouseState() const
{
    return Warehouse.GetState();
}

FNiumaSpatialContainerState UNiumaWarehouseSubsystem::GetWarehouseSnapshot() const
{
    return Warehouse.GetState();
}

ENiumaWarehouseOperationResult UNiumaWarehouseSubsystem::FindItem(
    const FGuid& InstanceId,
    FNiumaSpatialItemPlacement& OutPlacement) const
{
    return Warehouse.TryFindPlacement(
        InstanceId,
        OutPlacement,
        nullptr);
}

FNiumaWarehouseOperationResponse UNiumaWarehouseSubsystem::TryRelocateItem(
    const FGuid& InstanceId,
    FIntPoint NewOrigin,
    ENiumaItemOrientation NewOrientation)
{
    FString Error;

    FNiumaSpatialItemPlacement ExistingPlacement;

    const ENiumaWarehouseOperationResult FindResult = Warehouse.TryFindPlacement(
            InstanceId,
            ExistingPlacement,
            &Error);

    if (FindResult != ENiumaWarehouseOperationResult::Success)
    {
        return FNiumaWarehouseOperationResponse::MakeFailure(
            FindResult,
            Error);
    }

    FNiumaSpatialItemPlacement CandidatePlacement = ExistingPlacement;

    CandidatePlacement.Origin = NewOrigin;
    CandidatePlacement.Orientation = NewOrientation;

    /*
     * 提前准备成功响应。
     * 只有核心事务成功后才会真正返回或广播它。
     */
    const FNiumaWarehouseOperationResponse SuccessResponse =
        FNiumaWarehouseOperationResponse::MakeSuccess(
            CandidatePlacement);

    const int64 RevisionBefore = Warehouse.GetState().Revision;

    Error.Reset();

    const ENiumaWarehouseOperationResult RelocateResult =
        Warehouse.TryRelocate(
            InstanceId,
            NewOrigin,
            NewOrientation,
            ItemDefinitionResolver,
            &Error);

    if (RelocateResult != ENiumaWarehouseOperationResult::Success)
    {
        return FNiumaWarehouseOperationResponse::MakeFailure(
            RelocateResult,
            Error);
    }

    /*
     * 如果核心报告成功，候选 Placement 必须能够生成
     * 合法成功响应；否则说明内部契约已经不一致。
     */
    if (!SuccessResponse.IsSuccess())
    {
        return SuccessResponse;
    }

    const int64 RevisionAfter = Warehouse.GetState().Revision;

    /*
     * Revision 未变化表示目标与原 Placement 完全相同，
     * 属于成功无操作，不应通知 UI 刷新。
     */
    if (RevisionAfter != RevisionBefore)
    {
        OnWarehouseChanged.Broadcast(
            RevisionAfter,
            SuccessResponse);
    }

    return SuccessResponse;
}