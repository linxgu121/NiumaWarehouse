#pragma once

#include "CoreMinimal.h"

#include "NiumaNetwork/Result/NiumaAccountLoginResult.h"

/**
 * 将已经收到的登录 HTTP 响应转换成领域结果。
 *
 * 不依赖 IHttpResponse，方便进行自动化测试。
 * 这是模块内部类，因此放在 Private 且不添加 NIUMA_API。
 */
class FNiumaAccountHttpResponseMapper final
{
public:
	static FNiumaAccountLoginResult MapResponse(
		int32 HttpStatusCode,
		const FString& ResponseBody);

private:
	FNiumaAccountHttpResponseMapper() = delete;
};
