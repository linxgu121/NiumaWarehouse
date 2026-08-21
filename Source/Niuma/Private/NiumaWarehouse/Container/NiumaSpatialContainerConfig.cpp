#include "NiumaWarehouse/Container/NiumaSpatialContainerConfig.h"

namespace
{
    void SetContainerConfigError(FString* OutError,const TCHAR* Message)
    {
        if (OutError != nullptr)
        {
            *OutError = Message;
        }
    }

    bool IsSupportedContainerType(ENiumaItemContainerType ContainerType)
    {
        switch (ContainerType)
        {
        case ENiumaItemContainerType::Warehouse:
        case ENiumaItemContainerType::Backpack:
        case ENiumaItemContainerType::Corpse:
            return true;

        case ENiumaItemContainerType::None:
        default:
            return false;
        }
    }

    bool IsSupportedItemType(ENiumaItemType ItemType)
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
}

bool FNiumaSpatialContainerConfig::TryCreate(
    ENiumaItemContainerType InContainerType,
    int32 InWidth,
    int32 InHeight,
    const TSet<ENiumaItemType>& InAcceptedItemTypes,
    FNiumaSpatialContainerConfig& OutConfig,
    FString* OutError)
{
    FNiumaSpatialContainerConfig Candidate;

    Candidate.ContainerId = FGuid::NewGuid();
    Candidate.ContainerType = InContainerType;
    Candidate.Width = InWidth;
    Candidate.Height = InHeight;
    Candidate.AcceptedItemTypes = InAcceptedItemTypes;

    if (!Candidate.IsValid(OutError))
    {
        return false;
    }

    OutConfig = MoveTemp(Candidate);

    return true;
}

bool FNiumaSpatialContainerConfig::IsValid(FString* OutError) const
{
    if (!ContainerId.IsValid())
    {
        SetContainerConfigError(OutError,TEXT("空间容器的 ContainerId 无效"));

        return false;
    }

    if (!IsSupportedContainerType(ContainerType))
    {
        SetContainerConfigError(OutError,TEXT("空间容器包含无效的 ContainerType"));

        return false;
    }

    if (Width <= 0)
    {
        SetContainerConfigError(OutError,TEXT("空间容器的 Width 必须大于 0"));

        return false;
    }

    if (Height <= 0)
    {
        SetContainerConfigError(OutError,TEXT("空间容器的 Height 必须大于 0"));

        return false;
    }

    if (AcceptedItemTypes.IsEmpty())
    {
        SetContainerConfigError(OutError,TEXT("空间容器至少需要允许一种物品类型"));

        return false;
    }

    for (const ENiumaItemType ItemType :AcceptedItemTypes)
    {
        if (!IsSupportedItemType(ItemType))
        {
            SetContainerConfigError(OutError,TEXT("空间容器的接收范围包含"" None 或无效物品类型"));

            return false;
        }
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}
