#include "NiumaWarehouse/Subsystem/NiumaWarehouseSubsystem.h"

#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"

#include "NiumaWarehouse/Definitions/NiumaWarehouseDefinition.h"
#include "NiumaWarehouse/Settings/NiumaWarehouseSettings.h"
#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"
#include "NiumaNetwork/Session/NiumaAccountSessionSubsystem.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseHttpGateway.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseRelocateDtos.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotResult.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotConverter.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotApplier.h"
#include "NiumaNetwork/Session/NiumaAccountSessionState.h"
#include "NiumaNetwork/Result/NiumaRemoteOutcome.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseGrantDtos.h"

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

    /**
     * 把领域朝向枚举转换为服务端协议角度。
     * 非法枚举值返回 -1，由调用方同步拒绝。
     */
    int32 ToOrientationDegrees(ENiumaItemOrientation Orientation)
    {
        switch (Orientation)
        {
        case ENiumaItemOrientation::Degree0:
            return 0;

        case ENiumaItemOrientation::Degree90:
            return 90;

        case ENiumaItemOrientation::Degree180:
            return 180;

        case ENiumaItemOrientation::Degree270:
            return 270;

        default:
            return -1;
        }
    }
}

void UNiumaWarehouseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CancelPendingRemoteRequest();
    RemoteWarehousePlayerUid.Reset();

    UNiumaAccountSessionSubsystem* Session =
        Collection.InitializeDependency<
        UNiumaAccountSessionSubsystem>();

    AccountSession = Session;

    if (IsValid(Session))
    {
        AccountSessionChangedHandle =
            Session->OnSessionChanged.AddUObject(
                this,
                &UNiumaWarehouseSubsystem::
                HandleAccountSessionChanged);
    }

    SetRemoteStateInternal(
        ENiumaWarehouseRemoteState::Unauthenticated);

    InitializationError.Reset();

    if (!TryInitializeDefaultWarehouse(&InitializationError))
    {
        SetRemoteStateInternal(
            ENiumaWarehouseRemoteState::Error,
            InitializationError);

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
    if (UNiumaAccountSessionSubsystem* Session =
        AccountSession.Get();
        IsValid(Session) &&
        AccountSessionChangedHandle.IsValid())
    {
        Session->OnSessionChanged.Remove(
            AccountSessionChangedHandle);
    }

    AccountSessionChangedHandle.Reset();
    AccountSession.Reset();

    CancelPendingRemoteRequest();
    RemoteWarehousePlayerUid.Reset();

    OnWarehouseChanged.Clear();
    OnRemoteStateChanged.Clear();

    SetRemoteStateInternal(ENiumaWarehouseRemoteState::Unauthenticated);

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

    FNiumaSpatialContainer CandidateWarehouse;

    FString ContainerError;

    if (!CandidateWarehouse.TryInitializeEmpty(
        ContainerConfig,
        &ContainerError))
    {
        SetInitializationError(
            OutError,
            FString::Printf(
                TEXT("空仓库初始化失败：%s"),
                *ContainerError));

        return false;
    }

    Warehouse = MoveTemp(CandidateWarehouse);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}

uint64 UNiumaWarehouseSubsystem::BeginRemoteRequest(
    FString PlayerUid,
    ENiumaWarehouseRemoteState PendingState)
{
    check(IsInGameThread());
    check(!bRemoteRequestPending);
    check(!PlayerUid.IsEmpty());

    ++RemoteRequestGeneration;

    bRemoteRequestPending = true;

    RemoteWarehousePlayerUid = PlayerUid;
    ActiveRemoteRequestPlayerUid = MoveTemp(PlayerUid);

    SetRemoteStateInternal(PendingState);

    return RemoteRequestGeneration;
}

void UNiumaWarehouseSubsystem::RecoverAuthoritativeSnapshotAfterWriteFailure(
    ERemoteRequestKind RequestKind,
    const FString& FailureMessage)
{
    if (RequestKind != ERemoteRequestKind::Write)
    {
        return;
    }

    FString ReloadError;

    if (!RequestLoadWarehouse(ReloadError))
    {
        SetRemoteStateInternal(
            ENiumaWarehouseRemoteState::Error,
            FString::Printf(
                TEXT("%s；自动重载未能启动：%s"),
                *FailureMessage,
                *ReloadError));
    }
}

bool UNiumaWarehouseSubsystem::RequestLoadWarehouse(
    FString& OutError)
{
    OutError.Reset();

    if (!IsInGameThread())
    {
        OutError = TEXT("仓库加载请求必须从游戏线程发起");
        return false;
    }

    if (bRemoteRequestPending)
    {
        OutError =
            RemoteState == ENiumaWarehouseRemoteState::Writing
            ? TEXT("仓库修改正在提交")
            : TEXT("仓库快照正在加载");

        return false;
    }

    if (!Warehouse.IsInitialized())
    {
        OutError = TEXT("本地仓库尚未初始化");

        SetRemoteStateInternal(
            ENiumaWarehouseRemoteState::Error,
            OutError);

        return false;
    }

    UNiumaAccountSessionSubsystem* Session =
        AccountSession.Get();

    if (!IsValid(Session) ||
        !Session->IsAuthenticated())
    {
        FString ClearError;

        if (!TryClearRemoteWarehouse(&ClearError))
        {
            OutError = MoveTemp(ClearError);
            return false;
        }

        OutError = TEXT("当前账号尚未登录");
        return false;
    }

    FString PlayerUid = Session->GetPlayerUid();

    if (PlayerUid.IsEmpty())
    {
        OutError = TEXT("当前账号缺少有效 PlayerUid");

        SetRemoteStateInternal(
            ENiumaWarehouseRemoteState::Unauthenticated);

        return false;
    }


    const uint64 RequestGeneration = BeginRemoteRequest(
            MoveTemp(PlayerUid),
            ENiumaWarehouseRemoteState::Loading);

    FNiumaWarehouseHttpGateway::RequestSnapshot(
        Session,
        FNiumaWarehouseSnapshotCompleted::CreateWeakLambda(
            this,
            [this, RequestGeneration](
                const FNiumaWarehouseSnapshotResult& Result)
            {
                HandleSnapshotRequestCompleted(
                    RequestGeneration,
                    ERemoteRequestKind::Load,
                    Result);
            }));

    return true;
}

bool UNiumaWarehouseSubsystem::TryApplyRemoteSnapshot(
    const FNiumaWarehouseSnapshotDto& Snapshot,
    FString& OutError)
{
    const UNiumaWarehouseSettings* Settings =
        GetDefault<UNiumaWarehouseSettings>();

    if (Settings == nullptr)
    {
        OutError = TEXT("无法读取仓库项目设置");
        return false;
    }

    const UNiumaWarehouseDefinition* Definition =
        Settings
        ->DefaultWarehouseDefinition
        .LoadSynchronous();

    if (Definition == nullptr)
    {
        OutError = TEXT("无法加载默认仓库定义");
        return false;
    }

    FNiumaSpatialContainerConfig CandidateConfig;
    FNiumaSpatialContainerState CandidateState;

    if (!FNiumaWarehouseSnapshotConverter::TryConvert(
        Snapshot,
        *Definition,
        Settings->SupportedSnapshotSchemaVersion,
        Settings->ExpectedItemCatalogVersion,
        CandidateConfig,
        CandidateState,
        &OutError))
    {
        return false;
    }

    if (!FNiumaWarehouseSnapshotApplier::TryApply(
        CandidateConfig,
        CandidateState,
        ItemDefinitionResolver,
        Warehouse,
        &OutError))
    {
        return false;
    }

    OutError.Reset();
    return true;
}

void UNiumaWarehouseSubsystem::HandleSnapshotRequestCompleted(
    uint64 RequestGeneration,
    ERemoteRequestKind RequestKind,
    const FNiumaWarehouseSnapshotResult& Result)
{
    if (!bRemoteRequestPending ||
        RequestGeneration != RemoteRequestGeneration)
    {
        return;
    }

    const FString RequestedPlayerUid =
        MoveTemp(ActiveRemoteRequestPlayerUid);

    bRemoteRequestPending = false;

    UNiumaAccountSessionSubsystem* Session =
        AccountSession.Get();

    if (!IsValid(Session) ||
        !Session->IsAuthenticated() ||
        Session->GetPlayerUid() != RequestedPlayerUid)
    {
        TryClearRemoteWarehouse();
        return;
    }

    if (!Result.IsSuccess())
    {
        FString ErrorMessage =
            Result.GetMessage().TrimStartAndEnd();

        if (ErrorMessage.IsEmpty())
        {
            ErrorMessage = TEXT("仓库请求失败");
        }

        const bool bRevisionConflict =
            Result.GetServerCode() ==
            TEXT("WAREHOUSE_REVISION_CONFLICT");

        const ENiumaWarehouseRemoteState FailureState =
            bRevisionConflict
            ? ENiumaWarehouseRemoteState::Conflict
            : ENiumaWarehouseRemoteState::Error;

        // 先广播具体失败，让 UI 有机会显示原因。
        SetRemoteStateInternal(
            FailureState,
            ErrorMessage);

        const bool bRequiresAuthoritativeReload =
            RequestKind == ERemoteRequestKind::Write &&
            (
                bRevisionConflict ||
                Result.GetOutcome() ==
                ENiumaRemoteOutcome::TransportFailure ||
                Result.GetOutcome() ==
                ENiumaRemoteOutcome::ProtocolFailure
            );

        if (bRequiresAuthoritativeReload)
        {
            RecoverAuthoritativeSnapshotAfterWriteFailure(
                RequestKind,
                ErrorMessage);
        }

        return;
    }
    const FNiumaWarehouseSnapshotDto* Snapshot =
        Result.GetSnapshotData();

    if (Snapshot == nullptr)
    {
        const FString ErrorMessage = TEXT("成功响应缺少仓库快照");

        SetRemoteStateInternal(
            ENiumaWarehouseRemoteState::Error,
            ErrorMessage);

        RecoverAuthoritativeSnapshotAfterWriteFailure(
            RequestKind,
            ErrorMessage);

        return;
    }

    FString ApplyError;

    if (!TryApplyRemoteSnapshot(
        *Snapshot,
        ApplyError))
    {
        SetRemoteStateInternal(
            ENiumaWarehouseRemoteState::Error,
            ApplyError);

        RecoverAuthoritativeSnapshotAfterWriteFailure(
            RequestKind,
            ApplyError);

        return;
    }

    SetRemoteStateInternal(ENiumaWarehouseRemoteState::Ready);
}

bool UNiumaWarehouseSubsystem::RequestRelocateItem(
    const FGuid& InstanceId,
    FIntPoint NewOrigin,
    ENiumaItemOrientation NewOrientation,
    FString& OutError)
{
    OutError.Reset();

    if (!IsInGameThread())
    {
        OutError = TEXT("仓库重定位请求必须从游戏线程发起");
        return false;
    }

    if (bRemoteRequestPending)
    {
        OutError =
            RemoteState == ENiumaWarehouseRemoteState::Writing
            ? TEXT("仓库修改正在提交")
            : TEXT("仓库快照正在加载");

        return false;
    }

    /*
     * Ready 是提交的前提：
     * 覆盖已认证、已绑定账号、快照已应用
     * 且 PlayerUid 与当前会话一致。
     */
    if (!IsRemoteWarehouseReady())
    {
        OutError = TEXT("远端仓库尚未就绪");
        return false;
    }

    UNiumaAccountSessionSubsystem* Session =
        AccountSession.Get();

    if (!IsValid(Session))
    {
        OutError = TEXT("当前账号会话已失效");
        return false;
    }

    /*
     * 本地预览第一步：物品必须存在于当前镜像。
     */
    FString PreviewError;

    FNiumaSpatialItemPlacement ExistingPlacement;

    const ENiumaWarehouseOperationResult FindResult =
        Warehouse.TryFindPlacement(
            InstanceId,
            ExistingPlacement,
            &PreviewError);

    if (FindResult != ENiumaWarehouseOperationResult::Success)
    {
        OutError = MoveTemp(PreviewError);
        return false;
    }

    /*
     * 目标与当前 Placement 完全相同：
     * 服务端会按幂等空操作处理，
     * 客户端直接短路，节省一次往返，
     * 与 TryRelocateItem 的成功无操作语义一致。
     */
    if (ExistingPlacement.Origin == NewOrigin &&
        ExistingPlacement.Orientation == NewOrientation)
    {
        return true;
    }

    /*
     * 本地预览第二步：目标位置与朝向必须合法。
     * 预览失败只拒绝本次请求，
     * 不移动远端状态机，仓库镜像保持原样。
     */
    PreviewError.Reset();

    const ENiumaWarehouseOperationResult
        PreviewResult = Warehouse.CanRelocate(
            InstanceId,
            NewOrigin,
            NewOrientation,
            ItemDefinitionResolver,
            &PreviewError);

    if (PreviewResult !=
        ENiumaWarehouseOperationResult::Success)
    {
        OutError = MoveTemp(PreviewError);
        return false;
    }

    const int32 OrientationDegrees = ToOrientationDegrees(NewOrientation);

    if (OrientationDegrees < 0)
    {
        OutError = TEXT("重定位目标朝向无效");
        return false;
    }

    FNiumaWarehouseRelocateRequestDto RequestDto;

    RequestDto.InstanceId = InstanceId.ToString(
            EGuidFormats::DigitsWithHyphensLower);

    RequestDto.OriginX = NewOrigin.X;
    RequestDto.OriginY = NewOrigin.Y;

    RequestDto.OrientationDegrees = OrientationDegrees;

    /*
     * 受理这一刻的镜像 Revision 就是
     * 乐观并发的期望值，
     * 服务端只在版本一致时接受命令。
     */
    RequestDto.ExpectedRevision = Warehouse.GetState().Revision;

    FString PlayerUid = Session->GetPlayerUid();

    const uint64 RequestGeneration = BeginRemoteRequest(
            MoveTemp(PlayerUid),
            ENiumaWarehouseRemoteState::Writing);

    FNiumaWarehouseHttpGateway::RequestRelocate(
        Session,
        RequestDto,
        FNiumaWarehouseSnapshotCompleted::CreateWeakLambda(
            this,
            [this, RequestGeneration](
                const FNiumaWarehouseSnapshotResult& Result)
            {
                HandleSnapshotRequestCompleted(
                    RequestGeneration,
                    ERemoteRequestKind::Write,
                    Result);
            }));

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

ENiumaWarehouseRemoteState UNiumaWarehouseSubsystem::GetRemoteState() const
{
    return RemoteState;
}

FString UNiumaWarehouseSubsystem::GetRemoteError() const
{
    return RemoteError;
}

bool UNiumaWarehouseSubsystem::IsRemoteWarehouseReady() const
{
    const UNiumaAccountSessionSubsystem* Session =
        AccountSession.Get();

    if (!IsValid(Session) ||
        !Session->IsAuthenticated())
    {
        return false;
    }

    return
        RemoteState ==
        ENiumaWarehouseRemoteState::Ready &&
        Warehouse.IsInitialized() &&
        !RemoteWarehousePlayerUid.IsEmpty() &&
        Session->GetPlayerUid() ==
        RemoteWarehousePlayerUid;
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

ENiumaWarehouseOperationResult UNiumaWarehouseSubsystem::CanRelocateItem(
    const FGuid& InstanceId,
    FIntPoint NewOrigin,
    ENiumaItemOrientation NewOrientation) const
{
    return Warehouse.CanRelocate(
        InstanceId,
        NewOrigin,
        NewOrientation,
        ItemDefinitionResolver,
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

FNiumaWarehouseOperationResponse UNiumaWarehouseSubsystem::TryRemoveItem(
    const FGuid& InstanceId)
{
    /*
     * 在提交移除前准备成功响应。
     * 有效 ID 才能生成成功响应，但最终是否能够移除
     * 仍由空间容器判断。
     */
    const FNiumaWarehouseOperationResponse SuccessResponse =
        FNiumaWarehouseOperationResponse::MakeRemovalSuccess(
            InstanceId);

    FString Error;

    const ENiumaWarehouseOperationResult RemoveResult =
        Warehouse.TryRemove(
            InstanceId,
            &Error);

    if (RemoveResult !=
        ENiumaWarehouseOperationResult::Success)
    {
        return FNiumaWarehouseOperationResponse::MakeFailure(
            RemoveResult,
            Error);
    }

    /*
     * 核心移除成功意味着 InstanceId 一定有效，
     * 因此预先构造的移除成功响应也必须有效。
     */
    if (!SuccessResponse.IsSuccess())
    {
        return SuccessResponse;
    }

    /*
     * TryRemove 没有成功无操作：
     * 返回 Success 就代表 Placement 已移除，
     * Occupancy 已释放且 Revision 已增加一次。
     */
    OnWarehouseChanged.Broadcast(
        Warehouse.GetState().Revision,
        SuccessResponse);

    return SuccessResponse;
}

#if !UE_BUILD_SHIPPING

bool UNiumaWarehouseSubsystem::RequestGrantItemForDevelopment(
    const FString& ItemDefinitionId,
    int32 Count,
    FString& OutError)
{
    OutError.Reset();

    if (!IsInGameThread())
    {
        OutError = TEXT("开发物品发放请求必须从游戏线程发起");
        return false;
    }

    if (bRemoteRequestPending)
    {
        OutError =
            RemoteState == ENiumaWarehouseRemoteState::Writing
            ? TEXT("仓库修改正在提交")
            : TEXT("仓库快照正在加载");

        return false;
    }

    // 发放属于远端写操作，必须先有当前账号的权威镜像。
    if (!IsRemoteWarehouseReady())
    {
        OutError = TEXT("远端仓库尚未就绪");
        return false;
    }

    UNiumaAccountSessionSubsystem* Session =
        AccountSession.Get();

    if (!IsValid(Session))
    {
        OutError = TEXT("当前账号会话已失效");
        return false;
    }

    FString NormalizedItemDefinitionId =
        ItemDefinitionId.TrimStartAndEnd();

    if (NormalizedItemDefinitionId.IsEmpty())
    {
        OutError = TEXT("发放物品定义 ID 不能为空");
        return false;
    }

    if (NormalizedItemDefinitionId.Len() > 128)
    {
        OutError =
            TEXT("发放物品定义 ID 不能超过 128 个字符");
        return false;
    }

    if (Count <= 0)
    {
        OutError = TEXT("发放数量必须大于 0");
        return false;
    }

    FNiumaWarehouseGrantRequestDto RequestDto;
    RequestDto.ItemDefinitionId =
        MoveTemp(NormalizedItemDefinitionId);
    RequestDto.Count = Count;

    FString PlayerUid = Session->GetPlayerUid();

    const uint64 RequestGeneration =
        BeginRemoteRequest(
            MoveTemp(PlayerUid),
            ENiumaWarehouseRemoteState::Writing);

    FNiumaWarehouseHttpGateway::
        RequestGrantItemForDevelopment(
            Session,
            RequestDto,
            FNiumaWarehouseSnapshotCompleted::CreateWeakLambda(
                this,
                [this, RequestGeneration](
                    const FNiumaWarehouseSnapshotResult& Result)
                {
                    HandleSnapshotRequestCompleted(
                        RequestGeneration,
                        ERemoteRequestKind::Write,
                        Result);
                }));

    return true;
}

#endif

void UNiumaWarehouseSubsystem::SetRemoteStateInternal(
    ENiumaWarehouseRemoteState NewState,
    FString ErrorMessage)
{
    FString NewError;

    if (NewState == ENiumaWarehouseRemoteState::Error ||
        NewState == ENiumaWarehouseRemoteState::Conflict)
    {
        NewError = MoveTemp(ErrorMessage);
    }

    const bool bChanged =
        RemoteState != NewState ||
        RemoteError != NewError;

    RemoteState = NewState;
    RemoteError = MoveTemp(NewError);

    if (bChanged)
    {
        OnRemoteStateChanged.Broadcast(
            RemoteState,
            RemoteError);
    }
}

void UNiumaWarehouseSubsystem::CancelPendingRemoteRequest()
{
    // 淘汰已经发出的旧回调。
    ++RemoteRequestGeneration;

    bRemoteRequestPending = false;
    ActiveRemoteRequestPlayerUid.Reset();
}

bool UNiumaWarehouseSubsystem::TryClearRemoteWarehouse(
    FString* OutError)
{
    CancelPendingRemoteRequest();
    RemoteWarehousePlayerUid.Reset();

    FString ResetError;

    if (!TryInitializeDefaultWarehouse(&ResetError))
    {
        // 清理失败时也不能继续保留上一账号的数据。
        Warehouse = FNiumaSpatialContainer{};

        if (OutError != nullptr)
        {
            *OutError = ResetError;
        }

        SetRemoteStateInternal(
            ENiumaWarehouseRemoteState::Error,
            ResetError);

        UE_LOG(
            LogNiumaWarehouseSubsystem,
            Error,
            TEXT("清空账号仓库镜像失败：%s"),
            *ResetError);

        return false;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    SetRemoteStateInternal(
        ENiumaWarehouseRemoteState::Unauthenticated);

    return true;
}

void UNiumaWarehouseSubsystem::HandleAccountSessionChanged()
{
    UNiumaAccountSessionSubsystem* Session =
        AccountSession.Get();

    if (!IsValid(Session))
    {
        TryClearRemoteWarehouse();
        return;
    }

    const ENiumaAccountSessionState SessionState =
        Session->GetSessionState();

    /*
     * 登录期间保留旧仓库。
     * 因为重新登录失败时，账号系统会继续保留旧会话。
     */
    if (SessionState ==
        ENiumaAccountSessionState::LoggingIn)
    {
        return;
    }

    if (SessionState == ENiumaAccountSessionState::LoggedOut)
    {
        TryClearRemoteWarehouse();
        return;
    }

    const FString CurrentPlayerUid =
        Session->GetPlayerUid();

    if (CurrentPlayerUid.IsEmpty())
    {
        TryClearRemoteWarehouse();
        return;
    }

    /*
     * 第一次登录时尚未绑定仓库账号，不需要清理。
     * 只有已经存在账号绑定且身份不同才属于账号切换。
     */
    if (!RemoteWarehousePlayerUid.IsEmpty() &&
        RemoteWarehousePlayerUid != CurrentPlayerUid)
    {
        TryClearRemoteWarehouse();
    }
}


