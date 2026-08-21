#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouseRemoteDtos.generated.h"

/**
 * Java 仓库快照中的单个物品 Placement。
 *
 * 这是未经领域验证的网络数据，不能直接写入仓库容器。
 */
USTRUCT()
struct NIUMA_API FNiumaWarehousePlacementDto
{
	GENERATED_BODY()

    /**
     * Java UUID 字符串。
     * 后续 Converter 将其转换为 FGuid。
     */
    UPROPERTY()
    FString InstanceId;

    /**
     * 服务端稳定物品定义 ID，例如 Weapon_AK47。
     */
    UPROPERTY()
    FString ItemDefinitionId;

    /**
     * 物品数量。
     */
    UPROPERTY()
    int32 Count = 0;

    /**
     * 仓库逻辑格原点 X。
     */
    UPROPERTY()
    int32 OriginX = -1;

    /**
     * 仓库逻辑格原点 Y。
     */
    UPROPERTY()
    int32 OriginY = -1;

    /**
     * 服务端方向角度，只允许 0、90、180、270。
     */
    UPROPERTY()
    int32 OrientationDegrees = -1;

};

/**
 * GET /api/v1/game/warehouse 中的 data。
 */
USTRUCT()
struct NIUMA_API FNiumaWarehouseSnapshotDto
{
    GENERATED_BODY()

    /**
     * 仓库容器 UUID。
     */
    UPROPERTY()
    FString ContainerId;

    /**
     * 服务端仓库定义 ID，例如 PlayerWarehouse。
     */
    UPROPERTY()
    FString DefinitionId;

    UPROPERTY()
    int32 Width = 0;

    UPROPERTY()
    int32 Height = 0;

    /**
     * 服务端权威仓库版本。
     */
    UPROPERTY()
    int64 Revision = -1;

    UPROPERTY()
    int32 SchemaVersion = 0;

    UPROPERTY()
    int32 CatalogVersion = 0;

    UPROPERTY()
    TArray<FNiumaWarehousePlacementDto> Placements;
};

/**
 * Java ApiResponse<WarehouseSnapshotResponse>。
 *
 * success 字段仍然需要 Converter 手动解析，
 * 因为 UE 属性名 bSuccess 与 Java 的 success 不一致。
 */
USTRUCT()
struct NIUMA_API FNiumaWarehouseSnapshotResponseDto
{
    GENERATED_BODY()

    UPROPERTY()
    bool bSuccess = false;

    UPROPERTY()
    FString Code;

    UPROPERTY()
    FString Message;

    UPROPERTY()
    FNiumaWarehouseSnapshotDto Data;

    UPROPERTY()
    int64 Timestamp = 0;
};