#include "NiumaWarehouse/Definitions/NiumaWarehouseDefinition.h"

UNiumaWarehouseDefinition::UNiumaWarehouseDefinition()
{
    AcceptedItemTypes.Reserve(5);

    AcceptedItemTypes.Add(ENiumaItemType::Weapon);
    AcceptedItemTypes.Add(ENiumaItemType::Armor);
    AcceptedItemTypes.Add(ENiumaItemType::StorageItem);
    AcceptedItemTypes.Add(ENiumaItemType::Consumable);
    AcceptedItemTypes.Add(ENiumaItemType::Miscellaneous);
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "NiumaWarehouseDefinition"

EDataValidationResult
UNiumaWarehouseDefinition::IsDataValid(
    FDataValidationContext& Context) const
{
    EDataValidationResult Result =
        Super::IsDataValid(Context);

    if (Width <= 0)
    {
        Context.AddError(
            LOCTEXT("WidthInvalid","仓库 Width 必须大于 0。"));

        Result = EDataValidationResult::Invalid;
    }

    if (Height <= 0)
    {
        Context.AddError(
            LOCTEXT("HeightInvalid", "仓库 Height 必须大于 0。"));

        Result = EDataValidationResult::Invalid;
    }

    if (AcceptedItemTypes.IsEmpty())
    {
        Context.AddError(
            LOCTEXT(
                "AcceptedItemTypesEmpty",
                "仓库至少需要允许一种物品类型。"));

        Result = EDataValidationResult::Invalid;
    }

    if (AcceptedItemTypes.Contains(
        ENiumaItemType::None))
    {
        Context.AddError(
            LOCTEXT(
                "AcceptedItemTypesContainsNone",
                "AcceptedItemTypes 不能包含未定义类型 None。"));

        Result = EDataValidationResult::Invalid;
    }

    if (Result ==
        EDataValidationResult::NotValidated)
    {
        Result = EDataValidationResult::Valid;
    }

    return Result;
}

#undef LOCTEXT_NAMESPACE

#endif
