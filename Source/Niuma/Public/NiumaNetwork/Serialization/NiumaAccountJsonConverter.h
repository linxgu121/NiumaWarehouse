#pragma once

#include "CoreMinimal.h"

#include "NiumaNetwork/Dto/NiumaAccountRemoteDtos.h"


/*
 * 账号接口 JSON 转换器。
 * JSON 实现细节被限制在 Private cpp 中，
 * 外部模块只接触 FString 和明确的 DTO。
 */
class NIUMA_API FNiumaAccountJsonConverter final
{
public:
	/**
	 * 解析 Java 登录统一响应。
	 *
	 * 返回 true 只表示响应结构合法；
	 * 业务是否成功由 OutResponse.bSuccess 判断。
	 *
	 * 解析失败时不修改 OutResponse。
	 */
	static bool TryParseLoginResponse(
		const FString& ResponseJson,
		FNiumaAccountLoginResponseDto& OutResponse,
		FString* OutError = nullptr); 

private:
	FNiumaAccountJsonConverter() = delete;
};
