#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotApplier.h"

bool FNiumaWarehouseSnapshotApplier::TryApply(
    const FNiumaSpatialContainerConfig& Config,
    const FNiumaSpatialContainerState& State,
    const INiumaItemSpatialDefinitionResolver& Resolver,
    FNiumaSpatialContainer& InOutWarehouse,
    FString* OutError)
{
    FNiumaSpatialContainer CandidateWarehouse;
    FString CandidateError;

    if (!CandidateWarehouse.TryInitializeFromState(
        Config,
        State,
        Resolver,
        &CandidateError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("仓库快照应用失败：%s"),
                *CandidateError);
        }

        return false;
    }

    InOutWarehouse = MoveTemp(CandidateWarehouse);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}
