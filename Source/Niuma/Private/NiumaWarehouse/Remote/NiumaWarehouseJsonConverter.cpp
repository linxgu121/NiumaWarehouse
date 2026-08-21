#include "NiumaWarehouse/Remote/NiumaWarehouseJsonConverter.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
    bool Fail(FString* OutError, const FString& Error)
    {
        if (OutError != nullptr)
        {
            *OutError = Error;
        }

        return false;
    }

    bool HasExactField(
        const FJsonObject& JsonObject,
        const TCHAR* ExpectedFieldName)
    {
        for (const auto& Field : JsonObject.Values)
        {
            if (MakeStringView(Field.Key).Equals(
                ExpectedFieldName,
                ESearchCase::CaseSensitive))
            {
                return true;
            }
        }

        return false;
    }

    bool RequireExactFields(
        const FJsonObject& JsonObject,
        const TCHAR* const* RequiredFields,
        int32 RequiredFieldCount,
        const TCHAR* Context,
        FString* OutError)
    {
        for (int32 Index = 0;Index < RequiredFieldCount; ++Index)
        {
            if (!HasExactField(JsonObject,RequiredFields[Index]))
            {
                return Fail(
                    OutError,
                    FString::Printf(
                        TEXT("%s 缺少精确字段 %s"),
                        Context,
                        RequiredFields[Index]));
            }
        }

        return true;
    }

    bool IsSupportedOrientation(
        int32 OrientationDegrees)
    {
        switch (OrientationDegrees)
        {
        case 0:
        case 90:
        case 180:
        case 270:
            return true;

        default:
            return false;
        }
    }

    bool ValidateSnapshotJsonFields(const FJsonObject& DataObject,FString* OutError)
    {
        const TCHAR* SnapshotFields[] =
        {
            TEXT("containerId"),
            TEXT("definitionId"),
            TEXT("width"),
            TEXT("height"),
            TEXT("revision"),
            TEXT("schemaVersion"),
            TEXT("catalogVersion"),
            TEXT("placements")
        };

        if (!RequireExactFields(
            DataObject,
            SnapshotFields,
            UE_ARRAY_COUNT(SnapshotFields),
            TEXT("仓库快照 data"),
            OutError))
        {
            return false;
        }

        const TArray<TSharedPtr<FJsonValue>>*
            PlacementValues = nullptr;

        if (!DataObject.TryGetArrayField(
            TEXT("placements"),
            PlacementValues) ||
            PlacementValues == nullptr)
        {
            return Fail(
                OutError,
                TEXT("仓库快照 placements 必须是数组"));
        }

        const TCHAR* PlacementFields[] =
        {
            TEXT("instanceId"),
            TEXT("itemDefinitionId"),
            TEXT("count"),
            TEXT("originX"),
            TEXT("originY"),
            TEXT("orientationDegrees")
        };

        for (const TSharedPtr<FJsonValue>& Value :
            *PlacementValues)
        {
            if (!Value.IsValid() ||
                Value->Type != EJson::Object)
            {
                return Fail(
                    OutError,
                    TEXT("仓库快照 placements 只能包含对象"));
            }

            const TSharedPtr<FJsonObject>
                PlacementObject = Value->AsObject();

            if (!PlacementObject.IsValid() ||
                !RequireExactFields(
                    *PlacementObject,
                    PlacementFields,
                    UE_ARRAY_COUNT(PlacementFields),
                    TEXT("仓库 Placement"),
                    OutError))
            {
                return false;
            }
        }

        return true;
    }

    bool ValidateSnapshotDto(
        const FNiumaWarehouseSnapshotDto& Snapshot,
        FString* OutError)
    {
        if (Snapshot.ContainerId
            .TrimStartAndEnd()
            .IsEmpty())
        {
            return Fail(OutError,TEXT("仓库快照 ContainerId 不能为空"));
        }

        if (Snapshot.DefinitionId
            .TrimStartAndEnd()
            .IsEmpty())
        {
            return Fail(OutError,TEXT("仓库快照 DefinitionId 不能为空"));
        }

        if (Snapshot.Width <= 0 ||
            Snapshot.Height <= 0)
        {
            return Fail(OutError,TEXT("仓库快照尺寸必须大于 0"));
        }

        if (Snapshot.Revision < 0)
        {
            return Fail(OutError,TEXT("仓库快照 Revision 不能小于 0"));
        }

        if (Snapshot.SchemaVersion < 1)
        {
            return Fail(OutError,TEXT("仓库快照 SchemaVersion 必须大于等于 1"));
        }

        if (Snapshot.CatalogVersion < 1)
        {
            return Fail(OutError,TEXT("仓库快照 CatalogVersion 必须大于等于 1"));
        }

        const int64 CellCount =
            static_cast<int64>(Snapshot.Width) *
            static_cast<int64>(Snapshot.Height);

        if (Snapshot.Placements.Num() > CellCount)
        {
            return Fail(OutError,TEXT("仓库快照 Placement 数量超过逻辑格数量"));
        }

        TSet<FString> InstanceIds;

        for (const FNiumaWarehousePlacementDto& Placement :
            Snapshot.Placements)
        {
            const FString NormalizedInstanceId =
                Placement.InstanceId
                .TrimStartAndEnd()
                .ToLower();

            if (NormalizedInstanceId.IsEmpty())
            {
                return Fail(OutError,TEXT("仓库 Placement InstanceId 不能为空"));
            }

            if (InstanceIds.Contains(
                NormalizedInstanceId))
            {
                return Fail(OutError,TEXT("仓库快照包含重复的 InstanceId"));
            }

            InstanceIds.Add(NormalizedInstanceId);

            if (Placement.ItemDefinitionId
                .TrimStartAndEnd()
                .IsEmpty())
            {
                return Fail(OutError,TEXT("仓库 Placement ItemDefinitionId 不能为空"));
            }

            if (Placement.Count <= 0)
            {
                return Fail(OutError,TEXT("仓库 Placement Count 必须大于 0"));
            }

            if (Placement.OriginX < 0 ||
                Placement.OriginY < 0)
            {
                return Fail(OutError,TEXT("仓库 Placement 原点不能包含负坐标"));
            }

            if (!IsSupportedOrientation(
                Placement.OrientationDegrees))
            {
                return Fail(OutError,TEXT("仓库 Placement 方向必须是 0、90、180 或 270"));
            }
        }

        return true;
    }
}

bool FNiumaWarehouseJsonConverter::TryBuildGrantRequestJson(
    const FNiumaWarehouseGrantRequestDto& Request,
    FString& OutJson,
    FString* OutError)
{
    const FString NormalizedItemDefinitionId =
        Request.ItemDefinitionId.TrimStartAndEnd();

    if (NormalizedItemDefinitionId.IsEmpty())
    {
        return Fail(
            OutError,
            TEXT("发放请求 ItemDefinitionId 不能为空"));
    }

    /*
     * 与 Java GrantWarehouseItemRequest
     * 的 @Size(max = 128) 保持一致。
     */
    if (NormalizedItemDefinitionId.Len() > 128)
    {
        return Fail(
            OutError,
            TEXT("发放请求 ItemDefinitionId 不能超过 128 个字符"));
    }

    if (Request.Count <= 0)
    {
        return Fail(
            OutError,
            TEXT("发放请求 Count 必须大于 0"));
    }

    /*
     * 先写入局部候选字符串。
     * 只有完整序列化成功后才替换 OutJson，
     * 保证失败不会破坏调用者原本持有的数据。
     */
    FString CandidateJson;

    const TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(
            &CandidateJson);

    Writer->WriteObjectStart();

    Writer->WriteValue(
        TEXT("itemDefinitionId"),
        NormalizedItemDefinitionId);

    Writer->WriteValue(
        TEXT("count"),
        Request.Count);

    Writer->WriteObjectEnd();

    if (!Writer->Close())
    {
        return Fail(
            OutError,
            TEXT("无法序列化仓库物品发放请求"));
    }

    OutJson = MoveTemp(CandidateJson);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}

bool FNiumaWarehouseJsonConverter::TryBuildRelocateRequestJson(
    const FNiumaWarehouseRelocateRequestDto& Request,
    FString& OutJson,
    FString* OutError)
{
    const FString NormalizedInstanceId =
        Request.InstanceId.TrimStartAndEnd();

    FGuid ParsedInstanceId;

    if (!FGuid::ParseExact(
        NormalizedInstanceId,
        EGuidFormats::DigitsWithHyphens,
        ParsedInstanceId) ||
        !ParsedInstanceId.IsValid())
    {
        return Fail(
            OutError,
            TEXT("重定位请求 InstanceId 必须是非零标准 UUID"));
    }

    if (Request.OriginX < 0 ||
        Request.OriginY < 0)
    {
        return Fail(
            OutError,
            TEXT("重定位请求目标坐标不能为负数"));
    }

    if (!IsSupportedOrientation(
        Request.OrientationDegrees))
    {
        return Fail(
            OutError,
            TEXT("重定位请求方向必须是 0、90、180 或 270"));
    }

    if (Request.ExpectedRevision < 0)
    {
        return Fail(
            OutError,
            TEXT("重定位请求 ExpectedRevision 不能小于 0"));
    }

    FString CandidateJson;

    const TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(
            &CandidateJson);

    Writer->WriteObjectStart();

    Writer->WriteValue(
        TEXT("instanceId"),
        ParsedInstanceId.ToString(
            EGuidFormats::DigitsWithHyphensLower));

    Writer->WriteValue(
        TEXT("originX"),
        Request.OriginX);

    Writer->WriteValue(
        TEXT("originY"),
        Request.OriginY);

    Writer->WriteValue(
        TEXT("orientationDegrees"),
        Request.OrientationDegrees);

    Writer->WriteValue(
        TEXT("expectedRevision"),
        Request.ExpectedRevision);

    Writer->WriteObjectEnd();

    if (!Writer->Close())
    {
        return Fail(
            OutError,
            TEXT("无法序列化仓库重定位请求"));
    }

    OutJson = MoveTemp(CandidateJson);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}

bool FNiumaWarehouseJsonConverter::
TryParseSnapshotResponse(
    const FString& Json,
    FNiumaWarehouseSnapshotResponseDto& OutResponse,
    FString* OutError)
{
    TSharedPtr<FJsonObject> RootObject;

    const TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(Json);

    if (!FJsonSerializer::Deserialize(
        Reader,
        RootObject) ||
        !RootObject.IsValid())
    {
        return Fail(
            OutError,
            TEXT("仓库响应不是合法 JSON 对象"));
    }

    const TCHAR* RootFields[] =
    {
        TEXT("success"),
        TEXT("code"),
        TEXT("message"),
        TEXT("data"),
        TEXT("timestamp")
    };

    if (!RequireExactFields(
        *RootObject,
        RootFields,
        UE_ARRAY_COUNT(RootFields),
        TEXT("仓库响应"),
        OutError))
    {
        return false;
    }

    // 所有校验都在候选对象上完成，保证失败原子性。
    FNiumaWarehouseSnapshotResponseDto Candidate;

    if (!RootObject->TryGetBoolField(
        TEXT("success"),
        Candidate.bSuccess))
    {
        return Fail(OutError,TEXT("仓库响应 success 类型无效"));
    }

    if (!RootObject->TryGetStringField(
        TEXT("code"),
        Candidate.Code) ||
        Candidate.Code
        .TrimStartAndEnd()
        .IsEmpty())
    {
        return Fail(OutError,TEXT("仓库响应 code 不能为空"));
    }

    if (!RootObject->TryGetStringField(
        TEXT("message"),
        Candidate.Message))
    {
        return Fail(OutError,TEXT("仓库响应 message 类型无效"));
    }

    if (!RootObject->TryGetNumberField(
        TEXT("timestamp"),
        Candidate.Timestamp) ||
        Candidate.Timestamp <= 0)
    {
        return Fail(OutError,TEXT("仓库响应 timestamp 无效"));
    }

    const TSharedPtr<FJsonValue> DataValue =
        RootObject->TryGetField(TEXT("data"));

    if (!DataValue.IsValid())
    {
        return Fail(OutError,TEXT("仓库响应缺少 data"));
    }

    if (!Candidate.bSuccess)
    {
        if (Candidate.Code == TEXT("OK"))
        {
            return Fail(OutError,TEXT("仓库失败响应不能使用 OK 代码"));
        }

        if (DataValue->Type != EJson::Null)
        {
            return Fail(OutError,TEXT("仓库失败响应的 data 必须为 null"));
        }

        OutResponse = MoveTemp(Candidate);

        if (OutError != nullptr)
        {
            OutError->Reset();
        }

        return true;
    }

    if (Candidate.Code != TEXT("OK"))
    {
        return Fail(OutError,TEXT("仓库成功响应必须使用 OK 代码"));
    }

    const TSharedPtr<FJsonObject>* DataObject = nullptr;

    if (!RootObject->TryGetObjectField(
        TEXT("data"),
        DataObject) ||
        DataObject == nullptr ||
        !DataObject->IsValid())
    {
        return Fail(OutError,TEXT("仓库成功响应缺少合法的 data 对象"));
    }

    if (!ValidateSnapshotJsonFields(
        **DataObject,
        OutError))
    {
        return false;
    }

    FText ConversionError;

    if (!FJsonObjectConverter::JsonObjectToUStruct(
        DataObject->ToSharedRef(),
        &Candidate.Data,
        0,
        0,
        true,
        &ConversionError))
    {
        return Fail(OutError,TEXT("仓库响应 data 结构不符合协议"));
    }

    if (!ValidateSnapshotDto(
        Candidate.Data,
        OutError))
    {
        return false;
    }

    OutResponse = MoveTemp(Candidate);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}
