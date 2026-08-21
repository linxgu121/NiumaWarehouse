#include "NiumaNetwork/Session/NiumaAccountSessionSubsystem.h"

#include "HAL/PlatformTime.h"

#include "NiumaNetwork/Dto/NiumaAccountRemoteDtos.h"
#include "NiumaNetwork/Gateway/NiumaAccountHttpGateway.h"

namespace
{
	double GetCurrentTimeSeconds()
	{
		return FPlatformTime::Seconds();
	}

	bool RejectRequest(
		FString* OutError,
		const FString& Error)
	{
		if (OutError != nullptr)
		{
			*OutError = Error;
		}

		return false;
	}
}

void UNiumaAccountSessionSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CancelPendingLogin();
	RuntimeSession.Clear();
}

void UNiumaAccountSessionSubsystem::Deinitialize()
{
	// 使仍在网络层执行的旧回调失效。
	CancelPendingLogin();
	RuntimeSession.Clear();
	OnSessionChanged.Clear();

	Super::Deinitialize();
}

bool UNiumaAccountSessionSubsystem::RequestLogin(
	const FString& Username,
	const FString& Password,
	FNiumaAccountSessionLoginCompleted Completion,
	FString* OutError)
{
	if (!IsInGameThread())
	{
		return RejectRequest(OutError,TEXT("登录请求必须从游戏线程发起"));
	}

	if (!Completion.IsBound())
	{
		return RejectRequest(OutError,TEXT("登录请求必须绑定完成回调"));
	}

	if (bLoginPending)
	{
		return RejectRequest(OutError,TEXT("当前已有登录请求正在进行"));
	}

	// 在广播状态变化前复制凭证。
	// 监听者可能在广播过程中清空登录界面的文本框。
	FNiumaAccountLoginRequestDto LoginRequest;
	LoginRequest.Username = Username;
	LoginRequest.Password = Password;

	++LoginRequestGeneration;

	const uint64 RequestGeneration =
		LoginRequestGeneration;

	bLoginPending = true;

	ActiveLoginCompletion =
		MoveTemp(Completion);

	if (OutError != nullptr)
	{
		OutError->Reset();
	}

	OnSessionChanged.Broadcast();

	// 广播期间可能发生 Logout 或 Deinitialize。
	if (!bLoginPending ||
		LoginRequestGeneration != RequestGeneration)
	{
		return true;
	}

	FNiumaAccountHttpGateway::RequestLogin(
		LoginRequest,
		FNiumaAccountLoginCompleted::CreateWeakLambda(
			this,
			[this, RequestGeneration](
				const FNiumaAccountLoginResult& Result)
			{
				HandleLoginCompleted(
					RequestGeneration,
					Result);
			}));

	return true;
}

void UNiumaAccountSessionSubsystem::HandleLoginCompleted(
	uint64 RequestGeneration,
	const FNiumaAccountLoginResult& Result)
{
	if (!bLoginPending || RequestGeneration != LoginRequestGeneration)
	{
		// Logout、Deinitialize 或更新的请求已使它失效。
		return;
	}

	FNiumaAccountSessionLoginCompleted Completion =
		MoveTemp(ActiveLoginCompletion);

	bLoginPending = false;

	ENiumaRemoteOutcome Outcome = Result.GetOutcome();

	FString Code = Result.GetServerCode();

	FString Message = Result.GetMessage();

	if (Result.IsSuccess())
	{
		const FNiumaAccountLoginDataDto* LoginData =
			Result.GetLoginData();

		FString SessionError;

		if (LoginData == nullptr ||
			!RuntimeSession.TryEstablish(
				*LoginData,
				GetCurrentTimeSeconds(),
				&SessionError))
		{
			Outcome = ENiumaRemoteOutcome::ProtocolFailure;

			Code = TEXT("SESSION_INVALID_LOGIN_DATA");

			Message = MoveTemp(SessionError);

			if (Message.IsEmpty())
			{
				Message = TEXT("登录成功响应缺少有效会话数据");
			}
		}
	}

	// 登录失败不会清除原有会话。
	// 因而账号切换失败时，旧账号仍保持登录。
	OnSessionChanged.Broadcast();

	Completion.ExecuteIfBound(
		Outcome,
		Code,
		Message);
}

void UNiumaAccountSessionSubsystem::Logout()
{
	const bool bHadPendingLogin = bLoginPending;

	const bool bHadAuthenticatedSession =
		RuntimeSession.IsAuthenticated(
			GetCurrentTimeSeconds());

	FNiumaAccountSessionLoginCompleted
		CancelledCompletion = CancelPendingLogin();

	RuntimeSession.Clear();

	if (bHadPendingLogin || bHadAuthenticatedSession)
	{
		OnSessionChanged.Broadcast();
	}

	if (bHadPendingLogin)
	{
		CancelledCompletion.ExecuteIfBound(
			ENiumaRemoteOutcome::TransportFailure,
			TEXT("SESSION_LOGIN_CANCELLED"),
			TEXT("登录请求已取消"));
	}
}
ENiumaAccountSessionState UNiumaAccountSessionSubsystem::GetSessionState() const
{
	if (bLoginPending)
	{
		return ENiumaAccountSessionState::LoggingIn;
	}

	return RuntimeSession.IsAuthenticated(
		GetCurrentTimeSeconds())
		? ENiumaAccountSessionState::Authenticated
		: ENiumaAccountSessionState::LoggedOut;
}

bool UNiumaAccountSessionSubsystem::IsAuthenticated() const
{
	return RuntimeSession.IsAuthenticated(
		GetCurrentTimeSeconds());
}

FString UNiumaAccountSessionSubsystem::GetPlayerUid() const
{
	FString PlayerUid;

	RuntimeSession.TryGetPlayerUid(
		GetCurrentTimeSeconds(),
		PlayerUid);

	return PlayerUid;
}

bool UNiumaAccountSessionSubsystem::TryBuildAuthorizationHeader(
	FString& OutAuthorizationHeader) const
{
	return RuntimeSession.TryBuildAuthorizationHeader(
		GetCurrentTimeSeconds(),
		OutAuthorizationHeader);
}

FNiumaAccountSessionLoginCompleted UNiumaAccountSessionSubsystem::CancelPendingLogin()
{
	// 推进代数，使已经发出的旧 HTTP 回调失效。
	++LoginRequestGeneration;

	bLoginPending = false;

	return MoveTemp(ActiveLoginCompletion);
}