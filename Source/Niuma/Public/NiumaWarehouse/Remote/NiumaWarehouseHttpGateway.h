#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Remote/NiumaWarehouseSnapshotResult.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseRelocateDtos.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseGrantDtos.h"

class UNiumaAccountSessionSubsystem;

/**
 * 仓库 HTTP 网关。
 *
 * 负责携带当前账号的 Bearer Token，
 * 向 Java 后端读取权威仓库快照，
 *或提交返回权威快照的仓库命令。
 *
 * 不向上层暴露 IHttpRequest、IHttpResponse 或 Access Token。
 */
class NIUMA_API FNiumaWarehouseHttpGateway final
{
public:
    /**
     * 异步请求当前账号的仓库快照。
     *
     * AccountSession 只在函数入口处用于生成认证头，
     * Gateway 不保存也不捕获该指针。
     *
     * Completion 最多执行一次，并保证在游戏线程执行。
     */
    static void RequestSnapshot(
        const UNiumaAccountSessionSubsystem* AccountSession,
        FNiumaWarehouseSnapshotCompleted Completion);

    /**
     * 异步提交仓库物品重定位命令。
     * Java 成功后返回完整权威仓库快照，
     * 因此继续使用 FNiumaWarehouseSnapshotCompleted。
     * Gateway 不保存或异步捕获 AccountSession。
     */
    static void RequestRelocate(
        const UNiumaAccountSessionSubsystem* AccountSession,
        const FNiumaWarehouseRelocateRequestDto& Request,
        FNiumaWarehouseSnapshotCompleted Completion);

#if !UE_BUILD_SHIPPING

    /**
     * 向开发后端申请给当前账号发放物品。
     *
     * 该入口只用于本地开发和 UI 联调，
     * Shipping 构建中不会提供。
     *
     * Java 成功后返回完整权威仓库快照。
     */
    static void RequestGrantItemForDevelopment(
        const UNiumaAccountSessionSubsystem* AccountSession,
        const FNiumaWarehouseGrantRequestDto& Request,
        FNiumaWarehouseSnapshotCompleted Completion);

#endif

private:
    FNiumaWarehouseHttpGateway() = delete;
};