#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"


#include "NiumaWarehouse/Container/NiumaSpatialContainer.h"
#include "NiumaWarehouse/Definitions/NiumaAssetManagerItemSpatialDefinitionResolver.h"
#include "NiumaWarehouse/Item/NiumaItemInstance.h"
#include "NiumaWarehouse/Result/NiumaWarehouseOperationResponse.h"
#include "NiumaWarehouse/Container/NiumaSpatialContainerState.h"
#include "NiumaWarehouse/Result/NiumaWarehouseOperationResult.h"
#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"
#include "NiumaWarehouse/Type/NiumaItemOrientation.h"
#include "NiumaWarehouse/Type/NiumaWarehouseRemoteState.h"

#include "NiumaWarehouseSubsystem.generated.h"

/**
 * 仓库状态成功变化后的通知。
 * 不携带完整仓库数组，监听者可根据 Revision
 * 判断是否需要重新获取快照。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FNiumaWarehouseChangedSignature,
    int64,
    NewRevision,
    FNiumaWarehouseOperationResponse,
    OperationResponse);

class FNiumaWarehouseSnapshotResult;
class UNiumaAccountSessionSubsystem;
struct FNiumaWarehouseSnapshotDto;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FNiumaWarehouseRemoteStateChangedSignature,
    ENiumaWarehouseRemoteState,
    NewState,
    FString,
    ErrorMessage);

UCLASS()
class NIUMA_API UNiumaWarehouseSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    /**
    * GameInstance 创建 Subsystem 时由 UE 自动调用。
    */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /*
    * 是否进行仓库初始化
    */
    UFUNCTION(BlueprintPure,Category = "Niuma|Warehouse")
    bool IsWarehouseInitialized() const;

    /*
    * 获取初始化错误
    */
    UFUNCTION(BlueprintPure,Category = "Niuma|Warehouse")
    FString GetInitializationError() const;

    /**
    * 请求读取当前登录账号的权威仓库快照。
    *
    * 返回 true 只表示请求已被接受，
    * 不表示仓库已经加载成功。
    */
    UFUNCTION(BlueprintCallable, Category = "Niuma|Warehouse|Remote")
    bool RequestLoadWarehouse(FString& OutError);

    /**
     * 请求服务端原子重定位仓库物品。
     *
     * 先用本地镜像做预览验证：
     * - 物品不存在或目标非法时返回 false，远端状态保持 Ready 不变；
     * - 目标与当前 Placement 完全相同时属于
     *   成功无操作，直接返回 true，不发送网络请求，也不改变 Revision。
     *
     * 预览通过后携带当前镜像 Revision 异步提交。
     * 返回 true 只表示请求已被受理，
     * 不表示重定位已经提交成功。
     * 提交结果通过 OnRemoteStateChanged 观察：
     * - 成功后应用服务端返回的权威快照并回到 Ready；
     * - WAREHOUSE_REVISION_CONFLICT 进入 Conflict，
     *   需要重新调用 RequestLoadWarehouse；
     * - 其他失败进入 Error。
     */
    UFUNCTION(BlueprintCallable, Category = "Niuma|Warehouse|Remote")
    bool RequestRelocateItem(
        const FGuid& InstanceId,
        FIntPoint NewOrigin,
        ENiumaItemOrientation NewOrientation,
        FString& OutError);

    /**
     * 远端仓库生命周期状态发生变化。
     */
    UPROPERTY(BlueprintAssignable, Category = "Niuma|Warehouse|Remote")
    FNiumaWarehouseRemoteStateChangedSignature OnRemoteStateChanged;

    /**
    * 获取仓库与服务端的当前同步状态。
   */
    UFUNCTION(BlueprintPure,Category = "Niuma|Warehouse|Remote")
    ENiumaWarehouseRemoteState GetRemoteState() const;



    /**
     * 获取最近一次远端同步错误。
     *
     * 非 Error 和 Conflict 状态下返回空字符串。
     */
    UFUNCTION(BlueprintPure,Category = "Niuma|Warehouse|Remote")
    FString GetRemoteError() const;

    /**
     * 服务端仓库快照是否已经成功加载并应用。
     */
    UFUNCTION(BlueprintPure,Category = "Niuma|Warehouse|Remote")
    bool IsRemoteWarehouseReady() const;

    /**
     * 当前只向 C++ 提供只读引用。
     * 后面会为蓝图提供独立快照，避免暴露内部可变数组。
     * 获取仓库配置
     */
    const FNiumaSpatialContainerConfig& GetWarehouseConfig() const;

    /*
    * 获取仓库状态
    */
    const FNiumaSpatialContainerState& GetWarehouseState() const;

    /**
    * 获取当前仓库状态的独立副本。
    *
    * 调用者修改返回值不会影响仓库内部状态。
    */
    UFUNCTION(BlueprintPure, Category = "Niuma|Warehouse")
    FNiumaSpatialContainerState GetWarehouseSnapshot() const;

    /**
    * 根据 InstanceId 查询仓库中的物品 Placement。
    *
    * 成功时写入 OutPlacement；
    * 失败时保持 OutPlacement 原样。
    */
    UFUNCTION(BlueprintPure, Category = "Niuma|Warehouse")
    ENiumaWarehouseOperationResult FindItem(
        const FGuid& InstanceId,
        FNiumaSpatialItemPlacement& OutPlacement) const;

    /**
     * 自动寻找第一个合法位置并接收物品。
     *
     * 失败时仓库保持原样。
     */
    FNiumaWarehouseOperationResponse TryReceiveItem(const FNiumaItemInstance& Item);

    /**
    * 判断仓库物品能否移动到目标位置与方向。
    *
    * 只进行预览查询，不修改仓库状态，
    * 不增加 Revision，也不广播变化事件。
    */
    UFUNCTION(BlueprintPure, Category = "Niuma|Warehouse")
    ENiumaWarehouseOperationResult CanRelocateItem(
        const FGuid& InstanceId,
        FIntPoint NewOrigin,
        ENiumaItemOrientation NewOrientation) const;

    /**
     * 原子修改仓库物品的位置与方向。
     * 真实变化成功时广播一次；
     * 成功无操作时不广播。
     */
    FNiumaWarehouseOperationResponse TryRelocateItem(
        const FGuid& InstanceId,
        FIntPoint NewOrigin,
        ENiumaItemOrientation NewOrientation);

    /**
    * 根据 InstanceId 原子移除仓库物品。
    *
    * 成功时 Revision 增加一次并广播一次；
    * 失败时仓库保持原样且不广播。
    */
    FNiumaWarehouseOperationResponse TryRemoveItem(
        const FGuid& InstanceId);


    /**
     * 只有仓库成功发生业务变化后才广播。
     */
    UPROPERTY(BlueprintAssignable,Category = "Niuma|Warehouse")
    FNiumaWarehouseChangedSignature OnWarehouseChanged;

    /**
     * 结束 Subsystem 生命周期并清理订阅关系。
     */
    virtual void Deinitialize() override;

private:
    /*
    * 请求类型
    */
    enum class ERemoteRequestKind : uint8
    {
        Load,
        Relocate
    };


    /**
    * 当前仓库镜像与服务端的同步状态。
    */
    ENiumaWarehouseRemoteState RemoteState = ENiumaWarehouseRemoteState::Unauthenticated;

    /**
     * 最近一次远端错误或冲突信息。
     */
    FString RemoteError;

    /**
    * 从 Project Settings 加载默认仓库定义，
    * 并初始化空的空间容器。
    */
    bool TryInitializeDefaultWarehouse(FString* OutError);

    FNiumaSpatialContainer Warehouse;

    void RecoverAuthoritativeSnapshotAfterWriteFailure(
        ERemoteRequestKind RequestKind,
        const FString& FailureMessage);

    /**
     * 把 ItemDefinitionId 解析为真实空间数据。
     */
    FNiumaAssetManagerItemSpatialDefinitionResolver ItemDefinitionResolver;

    FString InitializationError;

    /**
    * 修改远端生命周期状态。
    *
    * Error 与 Conflict 可以保存诊断信息；
    * 进入其他状态时自动清除旧错误。
    */
    void SetRemoteStateInternal(
        ENiumaWarehouseRemoteState NewState,
        FString ErrorMessage = FString());
    
    void HandleSnapshotRequestCompleted(
        uint64 RequestGeneration,
        ERemoteRequestKind RequestKind,
        const FNiumaWarehouseSnapshotResult& Result);

    bool TryApplyRemoteSnapshot(
        const FNiumaWarehouseSnapshotDto& Snapshot,
        FString& OutError);

    void CancelPendingRemoteRequest();

    bool bRemoteRequestPending = false;

    uint64 RemoteRequestGeneration = 0;

    /**
     * 防止账号 A 的旧请求结果应用到账号 B。
     */
    FString ActiveRemoteRequestPlayerUid;

    /**
     * 当前仓库状态所属的账号。
     *
     * 请求开始时绑定，即使请求失败，
     * Error 状态仍然明确属于该账号。
     */
    FString RemoteWarehousePlayerUid;

    /**
     * 不拥有账号 Subsystem，只观察其生命周期。
     */
    TWeakObjectPtr<UNiumaAccountSessionSubsystem> AccountSession;

    FDelegateHandle AccountSessionChangedHandle;

    void HandleAccountSessionChanged();

    /**
     * 取消远端请求并用默认空仓库替换账号镜像。
     */
    bool TryClearRemoteWarehouse(FString* OutError = nullptr);
};