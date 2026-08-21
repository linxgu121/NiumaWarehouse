#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotConverter.h"

#include "Misc/Guid.h"

#include "NiumaWarehouse/Definitions/NiumaItemDefinition.h"
#include "NiumaWarehouse/Definitions/NiumaWarehouseDefinition.h"
#include "NiumaWarehouse/Item/NiumaItemInstance.h"
#include "NiumaWarehouse/Spatial/NiumaSpatialItemPlacement.h"
#include "NiumaWarehouse/Type/NiumaItemContainerType.h"
#include "NiumaWarehouse/Type/NiumaItemOrientation.h"

namespace
{
    bool Fail(
        FString* OutError,
        FString Message)
    {
        if (OutError != nullptr)
        {
            *OutError = MoveTemp(Message);
        }

        return false;
    }

    bool TryParseRequiredGuid(
        const FString& Text,
        const TCHAR* FieldName,
        FGuid& OutGuid,
        FString* OutError)
    {
        FGuid CandidateGuid;

        if (!FGuid::ParseExact(
            Text.TrimStartAndEnd(),
            EGuidFormats::DigitsWithHyphens,
            CandidateGuid) ||
            !CandidateGuid.IsValid())
        {
            return Fail(
                OutError,
                FString::Printf(
                    TEXT("%s 不是有效的非零 UUID"),
                    FieldName));
        }

        OutGuid = CandidateGuid;

        return true;
    }

    bool TryConvertOrientation(
        int32 OrientationDegrees,
        ENiumaItemOrientation& OutOrientation)
    {
        switch (OrientationDegrees)
        {
        case 0:
            OutOrientation =
                ENiumaItemOrientation::Degree0;
            return true;

        case 90:
            OutOrientation =
                ENiumaItemOrientation::Degree90;
            return true;

        case 180:
            OutOrientation =
                ENiumaItemOrientation::Degree180;
            return true;

        case 270:
            OutOrientation =
                ENiumaItemOrientation::Degree270;
            return true;

        default:
            return false;
        }
    }

    bool TryConvertPlacement(
        const FNiumaWarehousePlacementDto& PlacementDto,
        int32 PlacementIndex,
        FNiumaSpatialItemPlacement& OutPlacement,
        FString* OutError)
    {
        FGuid InstanceId;
        FString ConversionError;

        if (!TryParseRequiredGuid(
            PlacementDto.InstanceId,
            TEXT("InstanceId"),
            InstanceId,
            &ConversionError))
        {
            return Fail(
                OutError,
                FString::Printf(
                    TEXT("第 %d 条 Placement 无效：%s"),
                    PlacementIndex,
                    *ConversionError));
        }

        const FString StableItemId =
            PlacementDto.ItemDefinitionId.TrimStartAndEnd();

        if (StableItemId.IsEmpty())
        {
            return Fail(
                OutError,
                FString::Printf(
                    TEXT(
                        "第 %d 条 Placement 的 "
                        "ItemDefinitionId 为空"),
                    PlacementIndex));
        }

        FNiumaItemInstance CandidateItem;
        CandidateItem.InstanceId = InstanceId;
        CandidateItem.ItemDefinitionId =
            FPrimaryAssetId(
                UNiumaItemDefinition::AssetType,
                FName(*StableItemId));
        CandidateItem.Count = PlacementDto.Count;

        FString ItemError;

        if (!CandidateItem.IsValid(&ItemError))
        {
            return Fail(
                OutError,
                FString::Printf(
                    TEXT("第 %d 条 Placement 物品无效：%s"),
                    PlacementIndex,
                    *ItemError));
        }

        ENiumaItemOrientation Orientation;

        if (!TryConvertOrientation(
            PlacementDto.OrientationDegrees,
            Orientation))
        {
            return Fail(
                OutError,
                FString::Printf(
                    TEXT(
                        "第 %d 条 Placement 的方向"
                        "必须是 0、90、180 或 270"),
                    PlacementIndex));
        }

        FNiumaSpatialItemPlacement CandidatePlacement;
        FString PlacementError;

        if (!FNiumaSpatialItemPlacement::TryCreate(
            CandidateItem,
            FIntPoint(
                PlacementDto.OriginX,
                PlacementDto.OriginY),
            Orientation,
            CandidatePlacement,
            &PlacementError))
        {
            return Fail(
                OutError,
                FString::Printf(
                    TEXT("第 %d 条 Placement 无效：%s"),
                    PlacementIndex,
                    *PlacementError));
        }

        OutPlacement = MoveTemp(CandidatePlacement);

        return true;
    }
}

bool FNiumaWarehouseSnapshotConverter::TryConvert(
    const FNiumaWarehouseSnapshotDto& Snapshot,
    const UNiumaWarehouseDefinition& WarehouseDefinition,
    int32 SupportedSchemaVersion,
    int32 ExpectedCatalogVersion,
    FNiumaSpatialContainerConfig& OutConfig,
    FNiumaSpatialContainerState& OutState,
    FString* OutError)
{
    if (SupportedSchemaVersion < 1)
    {
        return Fail(
            OutError,
            TEXT("客户端支持的快照结构版本无效"));
    }

    if (ExpectedCatalogVersion < 1)
    {
        return Fail(
            OutError,
            TEXT("客户端物品目录版本无效"));
    }

    if (Snapshot.SchemaVersion !=
        SupportedSchemaVersion)
    {
        return Fail(
            OutError,
            FString::Printf(
                TEXT(
                    "仓库快照 SchemaVersion 不兼容："
                    "服务端=%d，客户端=%d"),
                Snapshot.SchemaVersion,
                SupportedSchemaVersion));
    }

    if (Snapshot.CatalogVersion !=
        ExpectedCatalogVersion)
    {
        return Fail(
            OutError,
            FString::Printf(
                TEXT("仓库快照 CatalogVersion 不兼容:服务端=%d，客户端=%d"),
                Snapshot.CatalogVersion,
                ExpectedCatalogVersion));
    }

    if (WarehouseDefinition.DefinitionId.IsNone())
    {
        return Fail(OutError,TEXT("本地仓库定义缺少稳定 DefinitionId"));
    }

    const FString SnapshotDefinitionId =
        Snapshot.DefinitionId.TrimStartAndEnd();

    if (!SnapshotDefinitionId.Equals(
        WarehouseDefinition.DefinitionId.ToString(),
        ESearchCase::CaseSensitive))
    {
        return Fail(
            OutError,
            FString::Printf(
                TEXT("仓库 DefinitionId 不匹配:服务端=%s，本地=%s"),
                *SnapshotDefinitionId,
                *WarehouseDefinition.DefinitionId.ToString()));
    }

    if (Snapshot.Width != WarehouseDefinition.Width ||
        Snapshot.Height != WarehouseDefinition.Height)
    {
        return Fail(
            OutError,
            FString::Printf(
                TEXT("仓库尺寸不匹配:服务端=%d×%d，本地=%d×%d"),
                Snapshot.Width,
                Snapshot.Height,
                WarehouseDefinition.Width,
                WarehouseDefinition.Height));
    }

    FGuid ContainerId;
    FString GuidError;

    if (!TryParseRequiredGuid(
        Snapshot.ContainerId,
        TEXT("ContainerId"),
        ContainerId,
        &GuidError))
    {
        return Fail(
            OutError,
            MoveTemp(GuidError));
    }

    FNiumaSpatialContainerConfig CandidateConfig;
    CandidateConfig.ContainerId = ContainerId;
    CandidateConfig.ContainerType =
        ENiumaItemContainerType::Warehouse;
    CandidateConfig.Width = Snapshot.Width;
    CandidateConfig.Height = Snapshot.Height;
    CandidateConfig.AcceptedItemTypes =
        WarehouseDefinition.AcceptedItemTypes;

    FString ConfigError;

    if (!CandidateConfig.IsValid(&ConfigError))
    {
        return Fail(
            OutError,
            FString::Printf(
                TEXT("候选仓库配置无效：%s"),
                *ConfigError));
    }

    FNiumaSpatialContainerState CandidateState;
    CandidateState.Revision = Snapshot.Revision;
    CandidateState.Placements.Reserve(
        Snapshot.Placements.Num());

    for (int32 Index = 0;
        Index < Snapshot.Placements.Num();
        ++Index)
    {
        FNiumaSpatialItemPlacement CandidatePlacement;

        if (!TryConvertPlacement(
            Snapshot.Placements[Index],
            Index,
            CandidatePlacement,
            OutError))
        {
            return false;
        }

        CandidateState.Placements.Add(
            MoveTemp(CandidatePlacement));
    }

    FString StateError;

    if (!CandidateState.IsStructurallyValid(
        &StateError))
    {
        return Fail(
            OutError,
            FString::Printf(
                TEXT("候选仓库状态无效：%s"),
                *StateError));
    }

    OutConfig = MoveTemp(CandidateConfig);
    OutState = MoveTemp(CandidateState);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}