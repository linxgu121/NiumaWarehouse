#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouseRemoteState.generated.h"

UENUM(BlueprintType)
enum class ENiumaWarehouseRemoteState : uint8
{
    /**
    * 当前没有有效账号会话。
    *
    * 默认值设为该状态，避免默认构造后
    * 被错误视为已经可以使用仓库。
    */
    Unauthenticated = 0 UMETA(DisplayName = "未认证"),

    /**
     * 正在请求并应用完整仓库快照。
     */
    Loading = 10 UMETA(DisplayName = "加载中"),

    /**
     * 已成功应用服务端快照，
     * 当前本地仓库镜像可以使用。
     */
    Ready = 20 UMETA(DisplayName = "已就绪"),

    /**
     * 正在向服务端提交仓库修改。
     */
    Writing = 30 UMETA(DisplayName = "提交中"),

    /**
     * 客户端 Revision 已经过期，
     * 必须重新加载完整快照。
     */
    Conflict = 40 UMETA(DisplayName = "版本冲突"),

    /**
     * 网络、协议、转换或快照应用失败。
     */
    Error = 250 UMETA(DisplayName = "错误")
};
