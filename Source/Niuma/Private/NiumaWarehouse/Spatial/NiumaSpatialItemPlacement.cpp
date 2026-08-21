#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"


namespace
{
    void SetPlacementError(FString* OutError, const TCHAR* Message)
    {
        if (OutError != nullptr)
        {
            *OutError = Message;
        }
    }

    bool IsValidOrientation(ENiumaItemOrientation Orientation)
    {
        switch (Orientation)
        {
        case ENiumaItemOrientation::Degree0:
        case ENiumaItemOrientation::Degree90:
        case ENiumaItemOrientation::Degree180:
        case ENiumaItemOrientation::Degree270:
            return true;

        default:
            return false;
        }
    }
}

bool FNiumaSpatialItemPlacement::TryCreate(
    const FNiumaItemInstance& InItem,
    FIntPoint InOrigin,
    ENiumaItemOrientation InOrientation,
    FNiumaSpatialItemPlacement& OutPlacement,
    FString* OutError)
{
    FNiumaSpatialItemPlacement Candidate;

    Candidate.Item = InItem;
    Candidate.Origin = InOrigin;
    Candidate.Orientation = InOrientation;

    if (!Candidate.IsValid(OutError))
    {
        return false;
    }

    // 全部规则成功后才提交正式输出。
    OutPlacement = Candidate;

    return true;
}

bool FNiumaSpatialItemPlacement::IsValid(FString* OutError) const
{
    if (!Item.IsValid(OutError))
    {
        return false;
    }

    if (Origin.X < 0 || Origin.Y < 0)
    {
        SetPlacementError(OutError, TEXT("物品放置原点不能包含负坐标"));

        return false;
    }

    if (!IsValidOrientation(Orientation))
    {
        SetPlacementError(OutError, TEXT("物品放置记录包含无效方向"));

        return false;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}