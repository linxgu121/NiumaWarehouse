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
    UFUNCTION(BlueprintCallable, Category = "Niuma|Warehouse")
    FNiumaWarehouseOperationResponse TryReceiveItem(const FNiumaItemInstance& Item);

    /**
     * 原子修改仓库物品的位置与方向。
     *
     * 真实变化成功时广播一次；
     * 成功无操作时不广播。
     */
    UFUNCTION(BlueprintCallable, Category = "Niuma|Warehouse")
    FNiumaWarehouseOperationResponse TryRelocateItem(
        const FGuid& InstanceId,
        FIntPoint NewOrigin,
        ENiumaItemOrientation NewOrientation);

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
    /**
    * 从 Project Settings 加载默认仓库定义，
    * 并初始化空的空间容器。
    */
    bool TryInitializeDefaultWarehouse(FString* OutError);

    FNiumaSpatialContainer Warehouse;

    /**
     * 把 ItemDefinitionId 解析为真实空间数据。
     */
    FNiumaAssetManagerItemSpatialDefinitionResolver ItemDefinitionResolver;

    FString InitializationError;
    
};