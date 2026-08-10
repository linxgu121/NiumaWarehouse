#include "NiumaWarehouse/Container/NiumaSpatialContainerState.h"

bool FNiumaSpatialContainerState::IsStructurallyValid(FString* OutError) const
{
    if (Revision < 0)
    {
        if (OutError != nullptr)
        {
            *OutError = TEXT("空间容器的 Revision 不能为负数");
        }

        return false;
    }

    TSet<FGuid> SeenInstanceIds;
    SeenInstanceIds.Reserve(Placements.Num());

    for (int32 Index = 0;Index < Placements.Num();++Index)
    {
        const FNiumaSpatialItemPlacement& Placement = Placements[Index];

        FString PlacementError;

        if (!Placement.IsValid(&PlacementError))
        {
            if (OutError != nullptr)
            {
                *OutError = FString::Printf(
                    TEXT(
                        "第 %d 条 Placement 无效：%s"),
                    Index,
                    *PlacementError);
            }

            return false;
        }

        const FGuid& InstanceId = Placement.Item.InstanceId;

        if (SeenInstanceIds.Contains(InstanceId))
        {
            if (OutError != nullptr)
            {
                *OutError = FString::Printf(
                    TEXT("第 %d 条 Placement 包含""重复的 InstanceId"),Index);
            }

            return false;
        }

        SeenInstanceIds.Add(InstanceId);
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;

}