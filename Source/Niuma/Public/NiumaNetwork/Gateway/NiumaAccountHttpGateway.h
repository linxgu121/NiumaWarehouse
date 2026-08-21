#pragma once

#include "CoreMinimal.h"

#include "NiumaNetwork/Dto/NiumaAccountRemoteDtos.h"
#include "NiumaNetwork/Result/NiumaAccountLoginResult.h"

/**
 * 账号 HTTP 网关。
 *
 * 对外不暴露 IHttpRequest 和 IHttpResponse。
 */
class NIUMA_API FNiumaAccountHttpGateway final
{
public:
	/**
	 * 向 Java 后端发送异步登录请求。
	 *
	 * Completion 最多执行一次。
	 */
	static void RequestLogin(
		const FNiumaAccountLoginRequestDto& LoginRequest,
		FNiumaAccountLoginCompleted Completion);

private:
	FNiumaAccountHttpGateway() = delete;
};
