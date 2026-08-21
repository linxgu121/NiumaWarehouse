#include "NiumaWarehouse/Remote/NiumaWarehouseHttpGateway.h"

#include "Async/Async.h"
#include "PlatformHttp.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "NiumaNetwork/Session/NiumaAccountSessionSubsystem.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseHttpResponseMapper.h"
#include "NiumaNetwork/Settings/NiumaNetworkSettings.h"
#include "NiumaWarehouse/Remote/NiumaWarehouseJsonConverter.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "NiumaWarehouse/Remote/NiumaWarehouseHttpGatewayTestHook.h"
#endif

namespace
{
#if WITH_DEV_AUTOMATION_TESTS

    FNiumaWarehouseSnapshotRequestHandlerForTesting
        SnapshotRequestHandlerForTesting;

    FNiumaWarehouseRelocateRequestHandlerForTesting
        RelocateRequestHandlerForTesting;

    FNiumaWarehouseGrantRequestHandlerForTesting
        GrantRequestHandlerForTesting;

#endif

    constexpr const TCHAR* WarehouseSnapshotPath =
        TEXT("/api/v1/game/warehouse");

    constexpr const TCHAR* WarehouseRelocatePath =
        TEXT("/api/v1/game/warehouse/items/relocate");

#if !UE_BUILD_SHIPPING

    constexpr const TCHAR* WarehouseGrantPath =
        TEXT("/api/v1/dev/warehouse/items/grant");

#endif

    /**
     * 防止 ProcessRequest 启动失败后，
     * HTTP 实现又触发完成委托造成重复回调。
     *
     * Complete 只允许在游戏线程调用。
     */
    class FNiumaWarehouseSnapshotCompletionState final
    {
    public:
        explicit FNiumaWarehouseSnapshotCompletionState(
            FNiumaWarehouseSnapshotCompleted InCompletion)
            : Completion(MoveTemp(InCompletion))
        {
        }

        void Complete(FNiumaWarehouseSnapshotResult Result)
        {
            check(IsInGameThread());

            if (bCompleted)
            {
                return;
            }

            bCompleted = true;

            FNiumaWarehouseSnapshotCompleted Callback = MoveTemp(Completion);

            Callback.ExecuteIfBound(Result);
        }

    private:
        bool bCompleted = false;

        FNiumaWarehouseSnapshotCompleted Completion;
    };

    bool TryBuildWarehouseUrl(
        const UNiumaNetworkSettings& Settings,
        const TCHAR* RequestPath,
        FString& OutUrl,
        FString& OutError)
    {
        if (RequestPath == nullptr ||
            RequestPath[0] != TEXT('/'))
        {
            OutError =
                TEXT("仓库请求路径必须以 / 开头");
            return false;
        }

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

        if (FPlatformHttp::GetUrlDomain(
            BaseUrl).IsEmpty())
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

        OutUrl = BaseUrl + RequestPath;
        OutError.Reset();

        return true;
    }

    bool TryPrepareAuthenticatedWarehouseRequest(
        const UNiumaAccountSessionSubsystem* AccountSession,
        const TCHAR* RequestPath,
        const TSharedRef<
        FNiumaWarehouseSnapshotCompletionState,
        ESPMode::ThreadSafe>& CompletionState,
        FString& OutUrl,
        FString& OutAuthorizationHeader,
        float& OutTimeoutSeconds)
    {
        if (!IsInGameThread())
        {
            AsyncTask(
                ENamedThreads::GameThread,
                [CompletionState]()
                {
                    CompletionState->Complete(
                        FNiumaWarehouseSnapshotResult::
                        MakeTransportFailure(
                            TEXT("仓库请求必须从游戏线程发起")));
                });

            return false;
        }

        if (!IsValid(AccountSession))
        {
            CompletionState->Complete(
                FNiumaWarehouseSnapshotResult::
                MakeTransportFailure(
                    TEXT("仓库请求缺少有效账号会话")));

            return false;
        }

        if (!AccountSession->TryBuildAuthorizationHeader(
            OutAuthorizationHeader))
        {
            CompletionState->Complete(
                FNiumaWarehouseSnapshotResult::
                MakeTransportFailure(
                    TEXT("当前账号会话没有有效 Bearer Token")));

            return false;
        }

        const UNiumaNetworkSettings* Settings =
            GetDefault<UNiumaNetworkSettings>();

        if (Settings == nullptr)
        {
            OutAuthorizationHeader.Reset();

            CompletionState->Complete(
                FNiumaWarehouseSnapshotResult::
                MakeTransportFailure(
                    TEXT("无法读取网络配置")));

            return false;
        }

        FString UrlError;

        if (!TryBuildWarehouseUrl(
            *Settings,
            RequestPath,
            OutUrl,
            UrlError))
        {
            OutAuthorizationHeader.Reset();

            CompletionState->Complete(
                FNiumaWarehouseSnapshotResult::
                MakeTransportFailure(
                    MoveTemp(UrlError)));

            return false;
        }

        OutTimeoutSeconds = FMath::Max(
            Settings->RequestTimeoutSeconds,
            1.0f);

        return true;
    }

    //响应处理函数
    void HandleSnapshotHttpComplete(
        const TSharedRef<
        FNiumaWarehouseSnapshotCompletionState,
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
                FNiumaWarehouseSnapshotResult::
                MakeTransportFailure(
                    TEXT("仓库请求未收到有效 HTTP 响应")));

            return;
        }

        const int32 HttpStatusCode =
            HttpResponse->GetResponseCode();

        const FString ResponseBody =
            HttpResponse->GetContentAsString();

        CompletionState->Complete(
            FNiumaWarehouseHttpResponseMapper::MapResponse(
                HttpStatusCode,
                ResponseBody));
    }
}

void FNiumaWarehouseHttpGateway::RequestSnapshot(
    const UNiumaAccountSessionSubsystem* AccountSession,
    FNiumaWarehouseSnapshotCompleted Completion)
{
    if (!ensureMsgf(
        Completion.IsBound(),
        TEXT("仓库快照请求必须绑定完成回调")))
    {
        return;
    }

    const TSharedRef<
        FNiumaWarehouseSnapshotCompletionState,
        ESPMode::ThreadSafe> CompletionState =
        MakeShared<
        FNiumaWarehouseSnapshotCompletionState,
        ESPMode::ThreadSafe>(
            MoveTemp(Completion));

    FString SnapshotUrl;
    FString AuthorizationHeader;
    float TimeoutSeconds = 0.0f;

    if (!TryPrepareAuthenticatedWarehouseRequest(
        AccountSession,
        WarehouseSnapshotPath,
        CompletionState,
        SnapshotUrl,
        AuthorizationHeader,
        TimeoutSeconds))
    {
        return;
    }

#if WITH_DEV_AUTOMATION_TESTS

    if (SnapshotRequestHandlerForTesting)
    {
        SnapshotRequestHandlerForTesting(
            MoveTemp(SnapshotUrl),
            MoveTemp(AuthorizationHeader),
            FNiumaWarehouseSnapshotCompleted::CreateLambda(
                [CompletionState](
                    const FNiumaWarehouseSnapshotResult& Result)
                {
                    CompletionState->Complete(Result);
                }));

        return;
    }

#endif

    const TSharedRef<
        IHttpRequest,
        ESPMode::ThreadSafe> HttpRequest =
        FHttpModule::Get().CreateRequest();

    HttpRequest->SetDelegateThreadPolicy(
        EHttpRequestDelegateThreadPolicy::
        CompleteOnGameThread);

    HttpRequest->SetURL(SnapshotUrl);
    HttpRequest->SetVerb(TEXT("GET"));

    HttpRequest->SetHeader(
        TEXT("Accept"),
        TEXT("application/json"));

    HttpRequest->SetHeader(
        TEXT("Authorization"),
        AuthorizationHeader);

    HttpRequest->SetTimeout(
        TimeoutSeconds);

    // IHttpRequest 已复制认证头，不再让局部变量保留 Token。
    AuthorizationHeader.Reset();

    HttpRequest->OnProcessRequestComplete()
        .BindLambda(
            [CompletionState](
                FHttpRequestPtr Request,
                FHttpResponsePtr Response,
                bool bSucceeded)
            {
                HandleSnapshotHttpComplete(
                    CompletionState,
                    MoveTemp(Request),
                    MoveTemp(Response),
                    bSucceeded);
            });

    if (!HttpRequest->ProcessRequest())
    {
        CompletionState->Complete(
            FNiumaWarehouseSnapshotResult::
            MakeTransportFailure(
                TEXT("仓库 HTTP 请求无法启动")));
    }
}

void FNiumaWarehouseHttpGateway::RequestRelocate(
    const UNiumaAccountSessionSubsystem* AccountSession,
    const FNiumaWarehouseRelocateRequestDto& Request,
    FNiumaWarehouseSnapshotCompleted Completion)
{
    if (!ensureMsgf(
        Completion.IsBound(),
        TEXT("仓库重定位请求必须绑定完成回调")))
    {
        return;
    }

    const TSharedRef<
        FNiumaWarehouseSnapshotCompletionState,
        ESPMode::ThreadSafe> CompletionState =
        MakeShared<
        FNiumaWarehouseSnapshotCompletionState,
        ESPMode::ThreadSafe>(
            MoveTemp(Completion));

    FString RelocateUrl;
    FString AuthorizationHeader;
    float TimeoutSeconds = 0.0f;

    if (!TryPrepareAuthenticatedWarehouseRequest(
        AccountSession,
        WarehouseRelocatePath,
        CompletionState,
        RelocateUrl,
        AuthorizationHeader,
        TimeoutSeconds))
    {
        return;
    }

    FString RequestBody;
    FString SerializationError;

    if (!FNiumaWarehouseJsonConverter::
        TryBuildRelocateRequestJson(
            Request,
            RequestBody,
            &SerializationError))
    {
        AuthorizationHeader.Reset();

        CompletionState->Complete(
            FNiumaWarehouseSnapshotResult::
            MakeProtocolFailure(
                0,
                MoveTemp(SerializationError)));

        return;
    }

#if WITH_DEV_AUTOMATION_TESTS

    if (RelocateRequestHandlerForTesting)
    {
        RelocateRequestHandlerForTesting(
            MoveTemp(RelocateUrl),
            MoveTemp(AuthorizationHeader),
            MoveTemp(RequestBody),
            FNiumaWarehouseSnapshotCompleted::CreateLambda(
                [CompletionState](
                    const FNiumaWarehouseSnapshotResult& Result)
                {
                    CompletionState->Complete(Result);
                }));

        return;
    }

#endif

    const TSharedRef<
        IHttpRequest,
        ESPMode::ThreadSafe> HttpRequest =
        FHttpModule::Get().CreateRequest();

    HttpRequest->SetDelegateThreadPolicy(
        EHttpRequestDelegateThreadPolicy::
        CompleteOnGameThread);

    HttpRequest->SetURL(RelocateUrl);
    HttpRequest->SetVerb(TEXT("POST"));

    HttpRequest->SetHeader(
        TEXT("Accept"),
        TEXT("application/json"));

    HttpRequest->SetHeader(
        TEXT("Content-Type"),
        TEXT("application/json"));

    HttpRequest->SetHeader(
        TEXT("Authorization"),
        AuthorizationHeader);

    HttpRequest->SetContentAsString(
        RequestBody);

    HttpRequest->SetTimeout(
        TimeoutSeconds);

    // IHttpRequest 已复制这些字符串。
    AuthorizationHeader.Reset();
    RequestBody.Reset();

    HttpRequest->OnProcessRequestComplete()
        .BindLambda(
            [CompletionState](
                FHttpRequestPtr HttpRequest,
                FHttpResponsePtr HttpResponse,
                bool bSucceeded)
            {
                HandleSnapshotHttpComplete(
                    CompletionState,
                    MoveTemp(HttpRequest),
                    MoveTemp(HttpResponse),
                    bSucceeded);
            });

    if (!HttpRequest->ProcessRequest())
    {
        CompletionState->Complete(
            FNiumaWarehouseSnapshotResult::
            MakeTransportFailure(
                TEXT("仓库重定位 HTTP 请求无法启动")));
    }
}

#if !UE_BUILD_SHIPPING

void FNiumaWarehouseHttpGateway::
RequestGrantItemForDevelopment(
    const UNiumaAccountSessionSubsystem* AccountSession,
    const FNiumaWarehouseGrantRequestDto& Request,
    FNiumaWarehouseSnapshotCompleted Completion)
{
    if (!ensureMsgf(
        Completion.IsBound(),
        TEXT("开发物品发放请求必须绑定完成回调")))
    {
        return;
    }

    const TSharedRef<
        FNiumaWarehouseSnapshotCompletionState,
        ESPMode::ThreadSafe> CompletionState =
        MakeShared<
        FNiumaWarehouseSnapshotCompletionState,
        ESPMode::ThreadSafe>(
            MoveTemp(Completion));

    FString GrantUrl;
    FString AuthorizationHeader;
    float TimeoutSeconds = 0.0f;

    // 复用仓库网关已有的线程、会话、Token、
    // BaseUrl 和超时检查。
    if (!TryPrepareAuthenticatedWarehouseRequest(
        AccountSession,
        WarehouseGrantPath,
        CompletionState,
        GrantUrl,
        AuthorizationHeader,
        TimeoutSeconds))
    {
        return;
    }

    FString RequestBody;
    FString SerializationError;

    if (!FNiumaWarehouseJsonConverter::
        TryBuildGrantRequestJson(
            Request,
            RequestBody,
            &SerializationError))
    {
        AuthorizationHeader.Reset();

        CompletionState->Complete(
            FNiumaWarehouseSnapshotResult::
            MakeProtocolFailure(
                0,
                MoveTemp(SerializationError)));

        return;
    }

#if WITH_DEV_AUTOMATION_TESTS

    // 自动化测试在这里拦截请求，
    // 因此不会真的连接 Java 后端。
    if (GrantRequestHandlerForTesting)
    {
        GrantRequestHandlerForTesting(
            MoveTemp(GrantUrl),
            MoveTemp(AuthorizationHeader),
            MoveTemp(RequestBody),
            FNiumaWarehouseSnapshotCompleted::CreateLambda(
                [CompletionState](
                    const FNiumaWarehouseSnapshotResult& Result)
                {
                    CompletionState->Complete(Result);
                }));

        return;
    }

#endif

    const TSharedRef<IHttpRequest,ESPMode::ThreadSafe> HttpRequest =
        FHttpModule::Get().CreateRequest();

    HttpRequest->SetDelegateThreadPolicy(
        EHttpRequestDelegateThreadPolicy::
        CompleteOnGameThread);

    HttpRequest->SetURL(GrantUrl);
    HttpRequest->SetVerb(TEXT("POST"));

    HttpRequest->SetHeader(
        TEXT("Accept"),
        TEXT("application/json"));

    HttpRequest->SetHeader(
        TEXT("Content-Type"),
        TEXT("application/json"));

    HttpRequest->SetHeader(
        TEXT("Authorization"),
        AuthorizationHeader);

    HttpRequest->SetContentAsString(
        RequestBody);

    HttpRequest->SetTimeout(
        TimeoutSeconds);

    // IHttpRequest 已复制这些字符串，
    // 尽早清除局部 Token 和请求正文。
    AuthorizationHeader.Reset();
    RequestBody.Reset();

    HttpRequest->OnProcessRequestComplete()
        .BindLambda(
            [CompletionState](
                FHttpRequestPtr HttpRequest,
                FHttpResponsePtr HttpResponse,
                bool bSucceeded)
            {
                HandleSnapshotHttpComplete(
                    CompletionState,
                    MoveTemp(HttpRequest),
                    MoveTemp(HttpResponse),
                    bSucceeded);
            });

    if (!HttpRequest->ProcessRequest())
    {
        CompletionState->Complete(
            FNiumaWarehouseSnapshotResult::
            MakeTransportFailure(
                TEXT("开发物品发放 HTTP 请求无法启动")));
    }
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

void NiumaWarehouseHttpGatewayTestHook::
SetSnapshotRequestHandler(
    FNiumaWarehouseSnapshotRequestHandlerForTesting InHandler)
{
    check(IsInGameThread());

    SnapshotRequestHandlerForTesting =
        MoveTemp(InHandler);
}

void NiumaWarehouseHttpGatewayTestHook::
ResetSnapshotRequestHandler()
{
    check(IsInGameThread());

    SnapshotRequestHandlerForTesting.Reset();
}

void NiumaWarehouseHttpGatewayTestHook::
SetRelocateRequestHandler(
    FNiumaWarehouseRelocateRequestHandlerForTesting
    InHandler)
{
    check(IsInGameThread());

    RelocateRequestHandlerForTesting =
        MoveTemp(InHandler);
}

void NiumaWarehouseHttpGatewayTestHook::
ResetRelocateRequestHandler()
{
    check(IsInGameThread());

    RelocateRequestHandlerForTesting.Reset();
}

void NiumaWarehouseHttpGatewayTestHook::SetGrantRequestHandler(
    FNiumaWarehouseGrantRequestHandlerForTesting InHandler)
{
    check(IsInGameThread());

    GrantRequestHandlerForTesting =
        MoveTemp(InHandler);
}

void NiumaWarehouseHttpGatewayTestHook::
ResetGrantRequestHandler()
{
    check(IsInGameThread());

    GrantRequestHandlerForTesting.Reset();
}

#endif
