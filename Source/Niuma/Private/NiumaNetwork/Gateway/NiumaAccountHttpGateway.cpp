#include "NiumaNetwork/Gateway/NiumaAccountHttpGateway.h"

#include "Async/Async.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "PlatformHttp.h"

#include "NiumaNetwork/Gateway/NiumaAccountHttpResponseMapper.h"
#include "NiumaNetwork/Settings/NiumaNetworkSettings.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "NiumaNetwork/Gateway/NiumaAccountHttpGatewayTestHook.h"
#endif

namespace
{
#if WITH_DEV_AUTOMATION_TESTS

	FNiumaAccountLoginRequestHandlerForTesting
		LoginRequestHandlerForTesting;

#endif
	
	constexpr const TCHAR* LoginPath = TEXT("/api/v1/auth/login");

	/**
	 * 防止 ProcessRequest 启动失败后，
	 * HTTP 实现又触发完成委托造成重复回调。
	 *
	 * Complete 只在游戏线程调用。
	 */
	class FNiumaLoginCompletionState final
	{
	public:
		explicit FNiumaLoginCompletionState(
			FNiumaAccountLoginCompleted InCompletion)
			: Completion(MoveTemp(InCompletion))
		{
		}

		void Complete(
			FNiumaAccountLoginResult Result)
		{
			if (bCompleted)
			{
				return;
			}

			bCompleted = true;

			FNiumaAccountLoginCompleted Callback =
				MoveTemp(Completion);

			Callback.ExecuteIfBound(Result);
		}

	private:
		bool bCompleted = false;

		FNiumaAccountLoginCompleted Completion;
	};

	bool TryBuildLoginUrl(
		const UNiumaNetworkSettings& Settings,
		FString& OutUrl,
		FString& OutError)
	{
		FString BaseUrl =
			Settings.BaseUrl.TrimStartAndEnd();

		const bool bUsesHttp =
			BaseUrl.StartsWith(
				TEXT("http://"),
				ESearchCase::IgnoreCase);

		const bool bUsesHttps =
			BaseUrl.StartsWith(
				TEXT("https://"),
				ESearchCase::IgnoreCase);

		if (!bUsesHttp && !bUsesHttps)
		{
			OutError =
				TEXT("服务器 BaseUrl 必须使用 HTTP 或 HTTPS");
			return false;
		}

#if UE_BUILD_SHIPPING
		if (!bUsesHttps)
		{
			OutError =
				TEXT("Shipping 构建只允许 HTTPS");
			return false;
		}
#endif

		if (FPlatformHttp::GetUrlDomain(BaseUrl).IsEmpty())
		{
			OutError =
				TEXT("服务器 BaseUrl 缺少有效域名");
			return false;
		}

		if (BaseUrl.Contains(TEXT("?")) ||
			BaseUrl.Contains(TEXT("#")))
		{
			OutError =
				TEXT("服务器 BaseUrl 不能包含查询参数或片段");
			return false;
		}

		while (BaseUrl.EndsWith(TEXT("/")))
		{
			BaseUrl.LeftChopInline(1);
		}

		OutUrl = BaseUrl + LoginPath;
		OutError.Reset();

		return true;
	}

	void HandleLoginHttpComplete(
		const TSharedRef<
		FNiumaLoginCompletionState,
		ESPMode::ThreadSafe>& CompletionState,
		FHttpRequestPtr HttpRequest,
		FHttpResponsePtr HttpResponse,
		bool bProcessedSuccessfully)
	{
		(void)HttpRequest;

		if (!bProcessedSuccessfully ||
			!HttpResponse.IsValid())
		{
			CompletionState->Complete(
				FNiumaAccountLoginResult::
				MakeTransportFailure(
					TEXT("登录请求未收到有效 HTTP 响应")));

			return;
		}

		const int32 HttpStatusCode =
			HttpResponse->GetResponseCode();


		// 该字符串可能包含 Token，不允许写入日志。
		const FString ResponseBody =
			HttpResponse->GetContentAsString();

		CompletionState->Complete(
			FNiumaAccountHttpResponseMapper::MapResponse(
				HttpStatusCode,
				ResponseBody));
	}
}

void FNiumaAccountHttpGateway::RequestLogin(
	const FNiumaAccountLoginRequestDto& LoginRequest,
	FNiumaAccountLoginCompleted Completion)
{
	if (!ensureMsgf(
		Completion.IsBound(),
		TEXT("登录请求必须绑定完成回调")))
	{
		return;
	}

	const TSharedRef<
		FNiumaLoginCompletionState,
		ESPMode::ThreadSafe> CompletionState =
		MakeShared<
		FNiumaLoginCompletionState,
		ESPMode::ThreadSafe>(
			MoveTemp(Completion));

	if (!IsInGameThread())
	{
		AsyncTask(
			ENamedThreads::GameThread,
			[CompletionState]()
			{
				CompletionState->Complete(
					FNiumaAccountLoginResult::
					MakeTransportFailure(
						TEXT("登录请求必须从游戏线程发起")));
			});

		return;
	}

#if WITH_DEV_AUTOMATION_TESTS

	if (LoginRequestHandlerForTesting)
	{
		LoginRequestHandlerForTesting(
			LoginRequest,
			FNiumaAccountLoginCompleted::CreateLambda(
				[CompletionState](
					const FNiumaAccountLoginResult& Result)
				{
					CompletionState->Complete(Result);
				}));

		return;
	}

#endif

	const UNiumaNetworkSettings* Settings =
		GetDefault<UNiumaNetworkSettings>();

	FString LoginUrl;
	FString UrlError;

	if (Settings == nullptr ||
		!TryBuildLoginUrl(
			*Settings,
			LoginUrl,
			UrlError))
	{
		CompletionState->Complete(
			FNiumaAccountLoginResult::
			MakeTransportFailure(
				MoveTemp(UrlError)));

		return;
	}

	FString RequestBody;

	if (!FJsonObjectConverter::
		UStructToJsonObjectString(
			LoginRequest,
			RequestBody,
			0,
			0,
			0,
			nullptr,
			false))
	{
		CompletionState->Complete(
			FNiumaAccountLoginResult::
			MakeTransportFailure(
				TEXT("无法生成登录请求 JSON")));

		return;
	}

	const TSharedRef<IHttpRequest,ESPMode::ThreadSafe> HttpRequest =
		FHttpModule::Get().CreateRequest();

	HttpRequest->SetDelegateThreadPolicy(
		EHttpRequestDelegateThreadPolicy::
		CompleteOnGameThread);

	HttpRequest->SetURL(LoginUrl);
	HttpRequest->SetVerb(TEXT("POST"));

	HttpRequest->SetHeader(
		TEXT("Accept"),
		TEXT("application/json"));

	HttpRequest->SetHeader(
		TEXT("Content-Type"),
		TEXT("application/json; charset=utf-8"));

	HttpRequest->SetTimeout(
		FMath::Max(
			Settings->RequestTimeoutSeconds,
			1.0f));

	HttpRequest->SetContentAsString(RequestBody);

	// 尽早清除本地明文密码 JSON；
	// IHttpRequest 内部仍需持有发送所需的数据。
	RequestBody.Reset();

	HttpRequest->OnProcessRequestComplete()
		.BindLambda(
			[CompletionState](
				FHttpRequestPtr Request,
				FHttpResponsePtr Response,
				bool bSucceeded)
			{
				HandleLoginHttpComplete(
					CompletionState,
					MoveTemp(Request),
					MoveTemp(Response),
					bSucceeded);
			});

	if (!HttpRequest->ProcessRequest())
	{
		CompletionState->Complete(
			FNiumaAccountLoginResult::
			MakeTransportFailure(
				TEXT("登录 HTTP 请求无法启动")));
	}
}

#if WITH_DEV_AUTOMATION_TESTS

void NiumaAccountHttpGatewayTestHook::
SetLoginRequestHandler(
	FNiumaAccountLoginRequestHandlerForTesting
	InHandler)
{
	check(IsInGameThread());

	LoginRequestHandlerForTesting =
		MoveTemp(InHandler);
}

void NiumaAccountHttpGatewayTestHook::
ResetLoginRequestHandler()
{
	check(IsInGameThread());

	LoginRequestHandlerForTesting.Reset();
}

#endif
