#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"

#include "NiumaWarehouse/Spatial/NiumaItemFootprint.h"
#include "NiumaWarehouse/Type/NiumaItemOrientation.h"
#include "NiumaWarehouse/Type/NiumaItemRotationPolicy.h"
#include "NiumaWarehouse/Type/NiumaItemType.h"

/**
 * 已经由上层解析完成的物品空间数据。
 *
 * Footprint 应当是请求方向下经过校验、
 * 旋转和规范化后的二维形状。
 */
struct NIUMA_API FNiumaResolvedItemSpatialData
{
    /**
     * 检查解析结果是否满足空间容器契约。
     */
    bool IsStructurallyValid(FString* OutError = nullptr) const;

    /**
     * 物品业务类型，用于容器接收范围检查。
     */
    ENiumaItemType ItemType = ENiumaItemType::None;

    /**
     * 物品的旋转许可。
     */
    ENiumaItemRotationPolicy RotationPolicy = ENiumaItemRotationPolicy::Fixed;

    /**
     * 请求方向下的规范化 Footprint。
     */
    FNiumaItemFootprint Footprint;
};

/**
 * 物品空间定义解析接口。
 * 采用依赖倒置(DIP)设计方法
 * 实现者可以从 Asset Manager、缓存或测试假对象中
 * 取得物品定义，但空间容器不关心数据来自哪里。
 */
class NIUMA_API INiumaItemSpatialDefinitionResolver
{
public:
    virtual ~INiumaItemSpatialDefinitionResolver() = default;

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
        FString* OutError = nullptr) const = 0;
};