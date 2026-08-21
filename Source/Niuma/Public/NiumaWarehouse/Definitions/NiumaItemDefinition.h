#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "NiumaWarehouse/Type/NiumaItemStackPolicy.h"
#include "NiumaWarehouse/Type/NiumaItemType.h"
#include "NiumaWarehouse/Spatial/NiumaItemFootprint.h"
#include "NiumaWarehouse/Type/NiumaItemRotationPolicy.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "NiumaItemDefinition.generated.h"

/**
 * 仓库物品的静态定义。
 * 每种物品对应一个 Primary Data Asset。
 */
UCLASS(BlueprintType)
class NIUMA_API UNiumaItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

    /**
    * 所有物品定义共用的 Primary Asset Type。
    */
    static const FPrimaryAssetType AssetType;

    /**
     * 使用稳定 ItemId 生成 PrimaryAssetId。
     * 资产文件名和所在目录改变时，
     * 只要 ItemId 不变，存档引用就保持不变。
     */
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

    // 编辑器资产质检，只在编辑器构建中参与编译
#if WITH_EDITOR
    /// <summary>
    /// 数据验证
    /// </summary>
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

    
    /**
    * 跨存档与业务系统使用的稳定物品 ID
    * 不能使用 DisplayName 充当主键
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|Warehouse|Identity")
    FName ItemId = NAME_None;

    /**
    * 给玩家和策划查看的本地化名称
    * 自定义名称如(AK47)
    */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Niuma|Warehouse|Identity")
    FText DisplayName;

    /**
    * 物品所属的业务分类(物品种类)
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|Warehouse|Classification")
    ENiumaItemType ItemType = ENiumaItemType::None;

    /**
    * 物品是否允许进行数量堆叠
    * 不表示多个物品在空间上的层叠。
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|Warehouse|Stack")
    ENiumaItemStackPolicy StackPolicy =ENiumaItemStackPolicy::NonStackable;

    /**
    * 单个数量堆允许包含的最大物品数量。
    */
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Niuma|Warehouse|Stack",meta = (ClampMin = "1",UIMin = "1"))
    int32 MaxStackCount = 1;

    /**
    * 物品在仓库平面中是否允许进行90度旋转。
    */
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Niuma|Warehouse|Spatial")
    ENiumaItemRotationPolicy RotationPolicy = ENiumaItemRotationPolicy::Fixed;

    /**
    *  物品在单个水平层中占用的二维逻辑格形状。
    */
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Niuma|Warehouse|Spatial")
    FNiumaItemFootprint Footprint;

};