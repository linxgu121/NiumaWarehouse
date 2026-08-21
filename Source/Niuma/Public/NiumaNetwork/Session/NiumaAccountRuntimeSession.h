#pragma once

#include "CoreMinimal.h"

#include "NiumaNetwork/Dto/NiumaAccountRemoteDtos.h"

/**
 * 运行期账号认证数据。
 *
 * 只保存在内存中，不保存密码，不进行磁盘持久化。
 */
class NIUMA_API FNiumaAccountRuntimeSession final
{
public:
	FNiumaAccountRuntimeSession() = default;

	FNiumaAccountRuntimeSession(
		const FNiumaAccountRuntimeSession&) = delete;

	FNiumaAccountRuntimeSession& operator=(
		const FNiumaAccountRuntimeSession&) = delete;

	/**
	 * 使用登录成功数据建立会话。
	 * 失败时保留原会话，保证操作原子性。
	 */
	bool TryEstablish(
		const FNiumaAccountLoginDataDto& LoginData,
		double CurrentTimeSeconds,
		FString* OutError = nullptr);

	void Clear();

	bool IsAuthenticated(double CurrentTimeSeconds) const;

	/**
	 * 失败时保持 OutAuthorizationHeader 原样。
	 */
	bool TryBuildAuthorizationHeader(
		double CurrentTimeSeconds,
		FString& OutAuthorizationHeader) const;

	/**
	 * 失败时保持 OutPlayerUid 原样。
	 */
	bool TryGetPlayerUid(
		double CurrentTimeSeconds,
		FString& OutPlayerUid) const;


private:
	FString AccessToken;
	FString PlayerUid;
	double ExpiresAtSeconds = 0.0;
};
