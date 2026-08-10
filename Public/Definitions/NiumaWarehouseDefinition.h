#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "NiumaWarehouse/Type/NiumaItemType.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "NiumaWarehouseDefinition.generated.h"

/**
 * 仓库的静态空间与接收规则配置。
 */
UCLASS(BlueprintType)
class NIUMA_API UNiumaWarehouseDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UNiumaWarehouseDefinition();

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

    /**
     * 仓库横向逻辑格数量。
     */
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Niuma|Warehouse|Spatial",meta = (ClampMin = "1", UIMin = "1"))
    int32 Width = 20;

    /**
     * 仓库纵向逻辑格数量。
     */
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Niuma|Warehouse|Spatial",meta = (ClampMin = "1", UIMin = "1"))
    int32 Height = 30;

    /**
     * 当前仓库允许接收的物品类型。
     * 集合没有业务顺序，并且不会保存重复类型。
     */
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Niuma|Warehouse|Acceptance")
    TSet<ENiumaItemType> AcceptedItemTypes;
};
