#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

#include "NiumaNetwork/Result/NiumaRemoteOutcome.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseRemoteDtos.h"

/**
 * Gateway 完成仓库快照请求后的最终网络结果。
 *
 * 只有 Success 才能读取 SnapshotData。
 */
class NIUMA_API FNiumaWarehouseSnapshotResult final
{
public:
    static FNiumaWarehouseSnapshotResult MakeSuccess(
        int32 InHttpStatusCode,
        FNiumaWarehouseSnapshotDto InSnapshotData);

    static FNiumaWarehouseSnapshotResult MakeBusinessFailure(
        int32 InHttpStatusCode,
        FString InServerCode,
        FString InMessage);

    static FNiumaWarehouseSnapshotResult MakeTransportFailure(
        FString InDiagnosticMessage);

    static FNiumaWarehouseSnapshotResult MakeProtocolFailure(
        int32 InHttpStatusCode,
        FString InDiagnosticMessage);

    bool IsSuccess() const;

    bool HasHttpResponse() const;

    ENiumaRemoteOutcome GetOutcome() const;

    int32 GetHttpStatusCode() const;

    const FString& GetServerCode() const;

    const FString& GetMessage() const;

    /**
     * 只有 Success 才返回有效指针。
     */
    const FNiumaWarehouseSnapshotDto* GetSnapshotData() const;

private:
    FNiumaWarehouseSnapshotResult() = default;

    ENiumaRemoteOutcome Outcome = ENiumaRemoteOutcome::TransportFailure;

    /**
     * 没有收到 HTTP 响应时为 0。
     */
    int32 HttpStatusCode = 0;

    /**
     * 只保存服务端稳定业务代码。
     * TransportFailure 与 ProtocolFailure 时为空。
     */
    FString ServerCode;

    /**
     * 服务端消息或本地安全诊断信息。
     */
    FString Message;

    FNiumaWarehouseSnapshotDto SnapshotData;
};

/**
 * 仓库快照请求只完成一次。
 *
 * Result 引用只在回调执行期间有效；
 * 调用者需要保留的数据必须自行复制。
 */
DECLARE_DELEGATE_OneParam(
    FNiumaWarehouseSnapshotCompleted,
    const FNiumaWarehouseSnapshotResult&);
