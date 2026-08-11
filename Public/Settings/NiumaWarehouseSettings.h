#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"

#include "NiumaWarehouseSettings.generated.h"

class UNiumaWarehouseDefinition;

/**
 * 仓库系统的项目级配置。
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Niuma Warehouse"))
class NIUMA_API UNiumaWarehouseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
    /**
     * 让该设置显示在 Project Settings 的 Game 分类中。
     */
    virtual FName GetCategoryName() const override;

    /**
     * 新游戏会话使用的默认仓库定义。
     *
     * 使用软引用，避免启动时无条件加载资产，
     * 同时避免在 C++ 中写死 Content 路径。
     */
    UPROPERTY(Config,EditAnywhere,Category = "Default Warehouse")
    TSoftObjectPtr<UNiumaWarehouseDefinition> DefaultWarehouseDefinition;
};
