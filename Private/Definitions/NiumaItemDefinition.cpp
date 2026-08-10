#include "NiumaWarehouse/Definitions/NiumaItemDefinition.h"

#if WITH_EDITOR

#include "NiumaWarehouse/Spatial/Tool/NiumaItemFootprintUtility.h"

// UE 本地化系统（Localization），用来给文件里的可翻译文本划定命名空间。
#define LOCTEXT_NAMESPACE "NiumaItemDefinition"

EDataValidationResult UNiumaItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    if (ItemId.IsNone())
    {
        Context.AddError(LOCTEXT("ItemIdIsNone", "物品定义的 ItemId 不能为空。"));

        Result = EDataValidationResult::Invalid;
    }

    if (ItemType == ENiumaItemType::None)
    {
        Context.AddError(LOCTEXT("ItemTypeIsNone","物品定义的 ItemType 不能为未定义。"));

        Result = EDataValidationResult::Invalid;
    }

    switch (StackPolicy)
    {
    case ENiumaItemStackPolicy::NonStackable:
        if (MaxStackCount != 1)
        {
            Context.AddError(
                LOCTEXT("NonStackableCountInvalid", "不可堆叠物品的 MaxStackCount 必须等于 1。"));

            Result = EDataValidationResult::Invalid;
        }
        break;

    case ENiumaItemStackPolicy::Stackable:
        if (MaxStackCount <= 1)
        {
            Context.AddError(
                LOCTEXT("StackableCountInvalid","可堆叠物品的 MaxStackCount 必须大于 1。"));

            Result = EDataValidationResult::Invalid;
        }
        break;

    default:
        Context.AddError(
            LOCTEXT("StackPolicyInvalid","物品定义包含无效的 StackPolicy。"));

        Result = EDataValidationResult::Invalid;
        break;
    }

    FNiumaItemFootprint NormalizedFootprint;
    FString FootprintError;

    if (!FNiumaItemFootprintUtility::TryNormalize(
        Footprint,
        NormalizedFootprint,
        &FootprintError))
    {
        Context.AddError(
            FText::Format(
                LOCTEXT("FootprintInvalid","物品定义的 Footprint 无效：{0}"),
                FText::FromString(FootprintError)));

        Result = EDataValidationResult::Invalid;
    }

    if (Result ==EDataValidationResult::NotValidated)
    {
        Result = EDataValidationResult::Valid;
    }

    
    return Result;
}

#undef LOCTEXT_NAMESPACE

#endif