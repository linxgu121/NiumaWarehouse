#pragma once

#include "CoreMinimal.h"

#include "NiumaAccountRemoteDtos.generated.h"

/*
*  POST /api/v1/auth/login 的请求数据
*  结构只在发送请求期间短暂存在，
*  Password 不允许写入日志或长期保存。
*/
USTRUCT()
struct NIUMA_API FNiumaAccountLoginRequestDto
{
	GENERATED_BODY()

	/**
	 * 对应 JSON 字段 username。
	 */
	UPROPERTY()
	FString Username;

	/**
     * 对应 JSON 字段 password。
     */
	UPROPERTY()
	FString Password;
};

/*
*  登录成功后，Java ApiResponse.data 中的数据
*  AccessToken 只交给账号会话 Subsystem 保存
*/
USTRUCT()
struct NIUMA_API FNiumaAccountLoginDataDto
{
	GENERATED_BODY()

	/**
	 * JWT Access Token。
	 */
	UPROPERTY()
	FString AccessToken;

	/**
	 * 当前固定为 Bearer。
	 */
	UPROPERTY()
	FString TokenType;

	/**
	 * Token 距离过期还剩多少秒。
	 */
	UPROPERTY()
	int64 ExpiresInSeconds = 0;

	/**
	 * 面向玩家展示的九位 UID。
	 */
	UPROPERTY()
	FString PlayerUid;

};

/**
 * Java 统一响应 ApiResponse<LoginAccountResponse>。
 *
 * 注意：不能直接使用 FJsonObjectConverter
 * 转换整个响应。UE 的 bSuccess 会被转换成
 * JSON 字段 bSuccess，而 Java 返回的是 success。
 *
 * 后续 Converter 将手动解析响应外壳，
 * 再使用 FJsonObjectConverter 转换 Data。
 */
USTRUCT()
struct NIUMA_API FNiumaAccountLoginResponseDto
{
	GENERATED_BODY()

	/**
	 * 对应 JSON 字段 success。
	 * 由后续 Converter 手动赋值。
	 */
	UPROPERTY()
	bool bSuccess = false;

	/**
	 * 稳定业务码，例如 OK、ACCOUNT_INVALID_CREDENTIALS。
	 */
	UPROPERTY()
	FString Code;

	/**
	 * 仅用于诊断或展示，不参与业务分支判断。
	 */
	UPROPERTY()
	FString Message;

	/**
	 * success=true 时才具有意义。
	 */
	UPROPERTY()
	FNiumaAccountLoginDataDto Data;

	/**
	 * Java 服务端生成响应时的 Unix 毫秒时间戳。
	 */
	UPROPERTY()
	int64 Timestamp = 0;
};
