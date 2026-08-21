#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Definitions/NiumaItemSpatialDefinitionResolver.h"


class UNiumaItemDefinition;

/**
 * 使用 UE Asset Manager 解析真实物品定义资产。
 *
 * Resolver 只负责：
 * - 根据 PrimaryAssetId 找到并加载定义；
 * - 根据方向生成规范化 Footprint；
 * - 复制空间容器需要的数据。
 */
class NIUMA_API FNiumaAssetManagerItemSpatialDefinitionResolver final : public INiumaItemSpatialDefinitionResolver
{
public:
    /**
     * 根据物品定义 ID 与方向解析空间数据。
     *
     * 成功时写入 OutData；
     * 失败时不应修改 OutData。
     */
    virtual bool TryResolve(
        const FPrimaryAssetId& ItemDefinitionId,
        ENiumaItemOrientation Orientation,
        FNiumaResolvedItemSpatialData& OutData,
        FString* OutError = nullptr) const override;

private:
    /**
     * 从 Asset Manager 中查找或同步加载物品定义。
     */
    const UNiumaItemDefinition* FindOrLoadDefinition(
        const FPrimaryAssetId& ItemDefinitionId,
        FString* OutError) const;
};