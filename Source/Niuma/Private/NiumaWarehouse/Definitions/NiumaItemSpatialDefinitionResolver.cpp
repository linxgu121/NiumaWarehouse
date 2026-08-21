#include "NiumaWarehouse/Definitions/NiumaItemSpatialDefinitionResolver.h"

#include "NiumaWarehouse/Spatial/Tool/NiumaItemFootprintUtility.h"

namespace
{
    bool IsSupportedResolvedItemType(ENiumaItemType ItemType)
    {
        switch (ItemType)
        {
        case ENiumaItemType::Weapon:
        case ENiumaItemType::Armor:
        case ENiumaItemType::StorageItem:
        case ENiumaItemType::Consumable:
        case ENiumaItemType::Miscellaneous:
            return true;

        case ENiumaItemType::None:
        default:
            return false;
        }
    }

    bool IsSupportedRotationPolicy(ENiumaItemRotationPolicy RotationPolicy)
    {
        switch (RotationPolicy)
        {
        case ENiumaItemRotationPolicy::Fixed:
        case ENiumaItemRotationPolicy::QuarterTurns:
            return true;

        default:
            return false;
        }
    }
}

bool FNiumaResolvedItemSpatialData::IsStructurallyValid(FString* OutError) const
{
    if (!IsSupportedResolvedItemType(ItemType))
    {
        if (OutError != nullptr)
        {
            *OutError = TEXT("解析结果包含无效的 ItemType");
        }

        return false;
    }

    if (!IsSupportedRotationPolicy(RotationPolicy))
    {
        if (OutError != nullptr)
        {
            *OutError =TEXT("解析结果包含无效的RotationPolicy");
        }

        return false;
    }

    FNiumaItemFootprint NormalizedFootprint;
    FString FootprintError;

    if (!FNiumaItemFootprintUtility::TryNormalize(
        Footprint,
        NormalizedFootprint,
        &FootprintError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(TEXT("解析结果的 Footprint 无效：%s"), *FootprintError);
        }

        return false;
    }

    if (!(Footprint.Cells == NormalizedFootprint.Cells))
    {
        if (OutError != nullptr)
        {
            *OutError =TEXT("解析结果的 Footprint必须已经规范化并稳定排序");
        }

        return false;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}