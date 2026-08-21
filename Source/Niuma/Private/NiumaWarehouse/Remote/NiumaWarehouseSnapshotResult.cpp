#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotResult.h"

FNiumaWarehouseSnapshotResult FNiumaWarehouseSnapshotResult::MakeSuccess(
    int32 InHttpStatusCode,
    FNiumaWarehouseSnapshotDto InSnapshotData)
{
    FNiumaWarehouseSnapshotResult Result;

    Result.Outcome = ENiumaRemoteOutcome::Success;
    Result.HttpStatusCode = InHttpStatusCode;
    Result.ServerCode = TEXT("OK");
    Result.SnapshotData = MoveTemp(InSnapshotData);

    return Result;
}

FNiumaWarehouseSnapshotResult FNiumaWarehouseSnapshotResult::MakeBusinessFailure(
    int32 InHttpStatusCode,
    FString InServerCode,
    FString InMessage)
{
    FNiumaWarehouseSnapshotResult Result;

    Result.Outcome = ENiumaRemoteOutcome::BusinessFailure;

    Result.HttpStatusCode = InHttpStatusCode;
    Result.ServerCode = MoveTemp(InServerCode);
    Result.Message = MoveTemp(InMessage);

    return Result;
}

FNiumaWarehouseSnapshotResult FNiumaWarehouseSnapshotResult::MakeTransportFailure(
    FString InDiagnosticMessage)
{
    FNiumaWarehouseSnapshotResult Result;

    Result.Outcome =
        ENiumaRemoteOutcome::TransportFailure;

    Result.Message = MoveTemp(InDiagnosticMessage);

    return Result;
}

FNiumaWarehouseSnapshotResult FNiumaWarehouseSnapshotResult::MakeProtocolFailure(
    int32 InHttpStatusCode,
    FString InDiagnosticMessage)
{
    FNiumaWarehouseSnapshotResult Result;

    Result.Outcome = ENiumaRemoteOutcome::ProtocolFailure;

    Result.HttpStatusCode = InHttpStatusCode;
    Result.Message = MoveTemp(InDiagnosticMessage);

    return Result;
}

bool FNiumaWarehouseSnapshotResult::IsSuccess() const
{
    return Outcome == ENiumaRemoteOutcome::Success;
}

bool FNiumaWarehouseSnapshotResult::HasHttpResponse() const
{
    return HttpStatusCode > 0;
}

ENiumaRemoteOutcome FNiumaWarehouseSnapshotResult::GetOutcome() const
{
    return Outcome;
}

int32 FNiumaWarehouseSnapshotResult::GetHttpStatusCode() const
{
    return HttpStatusCode;
}

const FString& FNiumaWarehouseSnapshotResult::GetServerCode() const
{
    return ServerCode;
}

const FString& FNiumaWarehouseSnapshotResult::GetMessage() const
{
    return Message;
}

const FNiumaWarehouseSnapshotDto* FNiumaWarehouseSnapshotResult::GetSnapshotData() const
{
    return IsSuccess()
        ? &SnapshotData
        : nullptr;
}