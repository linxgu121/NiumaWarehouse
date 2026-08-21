#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

#include "NiumaNetwork/Dto/NiumaAccountRemoteDtos.h"
#include "NiumaNetwork/Result/NiumaRemoteOutcome.h"

/*
* HTTP 网关完成一次登录请求后的最终结果。
* Token 只在 Success 时有效。
* 不允许把整个结果或 Token 写入日志。
*/
class NIUMA_API FNiumaAccountLoginResult final
{
public:
	static FNiumaAccountLoginResult MakeSuccess(
		int32 InHttpStatusCode,
		FNiumaAccountLoginDataDto InLoginData);

	static FNiumaAccountLoginResult MakeBusinessFailure(
		int32 InHttpStatusCode,
		FString InServerCode,
		FString InMessage);

	static FNiumaAccountLoginResult MakeTransportFailure(
		FString InDiagnosticMessage);

	static FNiumaAccountLoginResult MakeProtocolFailure(
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
	const FNiumaAccountLoginDataDto* GetLoginData() const;

private:
	FNiumaAccountLoginResult() = default;

	ENiumaRemoteOutcome Outcome = ENiumaRemoteOutcome::TransportFailure;

	/**
	 * 未收到 HTTP 响应时为 0。
	 */
	int32 HttpStatusCode = 0;

	/**
	 * 后端返回的稳定代码。
	 * TransportFailure 和 ProtocolFailure 时为空。
	 */
	FString ServerCode;

	/**
	 * 业务失败消息或本地诊断信息。
	 */
	FString Message;

	FNiumaAccountLoginDataDto LoginData;
};

/**
 * 登录请求只完成一次。
 *
 * Result 引用仅在回调执行期间有效，
 * 接收者需要保存的数据必须自行复制。
 */
DECLARE_DELEGATE_OneParam(
	FNiumaAccountLoginCompleted,
	const FNiumaAccountLoginResult&);