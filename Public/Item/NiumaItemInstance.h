#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "UObject/PrimaryAssetId.h"

#include "NiumaItemInstance.generated.h"

/**
 * 玩家实际拥有的一份物品实例。
 *
 * 物品静态属性由 ItemDefinitionId 指向的定义资产提供；
 * 本结构只保存每份物品独有的运行时数据。
 */
USTRUCT(BlueprintType)
struct NIUMA_API FNiumaItemInstance
{
	GENERATED_BODY()

public:

    /**
     * 创建一个新的物品实例。
     * 只有成功时才修改 OutItemInstance。
     */
    static bool TryCreate(
        const FPrimaryAssetId& InItemDefinitionId,
        int32 InCount,
        FNiumaItemInstance& OutItemInstance,
        FString* OutError = nullptr);

    /**
     * 检查实例自身的数据是否合法。
     * 不负责加载物品定义资产。
     */
    bool IsValid(FString* OutError = nullptr) const;

    /**
     * 该物品实例的唯一标识。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ItemInstance")
    FGuid InstanceId;

    /**
     * 对应的物品静态定义资产 ID。
     * 不直接持有 UNiumaItemDefinition 指针。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ItemInstance")
    FPrimaryAssetId ItemDefinitionId;

    /**
     * 当前数量堆中包含的物品数量。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Niuma|Warehouse|ItemInstance")
    int32 Count = 1;


};
