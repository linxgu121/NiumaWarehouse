#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "NiumaWarehouse/Container/NiumaSpatialContainer.h"

#include "NiumaWarehouseSubsystem.generated.h"

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

private:
    /**
    * 从 Project Settings 加载默认仓库定义，
    * 并初始化空的空间容器。
    */
    bool TryInitializeDefaultWarehouse(FString* OutError);

    FNiumaSpatialContainer Warehouse;

    FString InitializationError;
};