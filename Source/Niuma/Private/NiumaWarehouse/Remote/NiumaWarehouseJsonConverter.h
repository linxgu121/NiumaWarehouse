#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Remote/NiumaWarehouseRemoteDtos.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseRelocateDtos.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseGrantDtos.h"

/**
 * Java 仓库 HTTP 响应 JSON 转换器。
 *
 * 只负责：
 * - 解析统一 ApiResponse 外壳
 * - 校验成功/失败响应结构
 * - 生成原始仓库快照 DTO
 *
 * 不负责：
 * - FGuid 转换
 * - FPrimaryAssetId 转换
 * - 构造 FNiumaSpatialContainerState
 * - 加载物品定义资产
 */
class FNiumaWarehouseJsonConverter final
{
public:

    /**
    * 把开发物品发放请求序列化为 Java 接口要求的 JSON。
    *
    * 成功时写入 OutJson；
    * 失败时保持 OutJson 原样。
    */
    static bool TryBuildGrantRequestJson(
        const FNiumaWarehouseGrantRequestDto& Request,
        FString& OutJson,
        FString* OutError = nullptr);

    /**
     * 把重定位请求 DTO 序列化为 Java 接口要求的 JSON。
     *
     * 成功时写入 OutJson；
     * 失败时保持 OutJson 原样。
     */
    static bool TryBuildRelocateRequestJson(
        const FNiumaWarehouseRelocateRequestDto& Request,
        FString& OutJson,
        FString* OutError = nullptr);

    /**
     * 解析 GET /api/v1/game/warehouse 的响应正文。
     *
     * 只有全部字段通过协议校验后才修改 OutResponse。
     * 失败时保持 OutResponse 原样，并通过 OutError
     * 返回不包含玩家隐私数据的诊断信息。
     */
    static bool TryParseSnapshotResponse(
        const FString& Json,
        FNiumaWarehouseSnapshotResponseDto& OutResponse,
        FString* OutError = nullptr);


private:
    FNiumaWarehouseJsonConverter() = delete;
};