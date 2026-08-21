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

   /**
    * 当前客户端支持的仓库快照结构版本。
    *
    * 修改该值不会自动迁移旧数据，
    * 只表示客户端允许接收哪个服务端版本。
    */
	UPROPERTY(
		Config,
		EditAnywhere,
		Category = "Remote Compatibility",
		meta = (ClampMin = "1", UIMin = "1"))
	int32 SupportedSnapshotSchemaVersion = 1;

	/**
	 * 当前客户端物品定义目录版本。
	 *
	 * 服务端返回不同版本时必须拒绝应用快照，
	 * 防止客户端使用错误 Footprint 重建仓库。
	 */
	UPROPERTY(
		Config,
		EditAnywhere,
		Category = "Remote Compatibility",
		meta = (ClampMin = "1", UIMin = "1"))
	int32 ExpectedItemCatalogVersion = 1;
};
