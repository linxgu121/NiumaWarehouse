#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "NiumaNetwork/Result/NiumaRemoteOutcome.h"
#include "NiumaNetwork/Result/NiumaAccountLoginResult.h"
#include "NiumaNetwork/Session/NiumaAccountRuntimeSession.h"
#include "NiumaNetwork/Session/NiumaAccountSessionState.h"

#include "NiumaAccountSessionSubsystem.generated.h"

/**
 * 一次登录操作完成后的安全结果。
 *
 * 不向调用者返回 Access Token。
 */
DECLARE_DELEGATE_ThreeParams(
	FNiumaAccountSessionLoginCompleted,
	ENiumaRemoteOutcome,
	const FString&,
	const FString&);

/**
 * 登录中、已登录、未登录之间发生变化。
 */
DECLARE_MULTICAST_DELEGATE(FNiumaAccountSessionChanged);

UCLASS()
class NIUMA_API UNiumaAccountSessionSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/*
	* 子系统诞生时重置所有状态，防止编辑器热重载后脏数据残留
	*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/*
	* 子系统销毁时取消网络回调、清空会话、解绑委托，防止野指针崩溃
	*/
	virtual void Deinitialize() override;

	/**
	 * 发起登录请求。
	 *
	 * 返回 true 只表示请求已被接受，不表示登录成功。
	 * 密码不会被保存进 Subsystem。
	 * 接收 UI 输入，校验条件 → 加锁 → 广播状态 → 发 HTTP → 挂回调
	 */
	bool RequestLogin(
		const FString& Username,
		const FString& Password,
		FNiumaAccountSessionLoginCompleted Completion,
		FString* OutError = nullptr);

	/*
	* 安全登出
	*/
	void Logout();

	UFUNCTION(BlueprintPure,Category = "Niuma|Account")
	ENiumaAccountSessionState GetSessionState() const;

	UFUNCTION(BlueprintPure,Category = "Niuma|Account")
	bool IsAuthenticated() const;

	UFUNCTION(BlueprintPure,Category = "Niuma|Account")
	FString GetPlayerUid() const;

	/**
	 * 只向需要访问受保护 API 的 C++ Gateway 开放。
	 * 不反射给蓝图，避免 UI 直接接触 Token。
	 */
	bool TryBuildAuthorizationHeader(FString& OutAuthorizationHeader) const;

	/**
    * RequestLogin、登录完成或 Logout 引发的可观察状态变化。
    *
    *Token 自然过期不进行 Tick 广播；
    * GetSessionState、IsAuthenticated 和认证头查询会实时判定过期。
    */
	FNiumaAccountSessionChanged OnSessionChanged;

private:
	/*
	* HTTP 网络回调回来后，安全地处理结果、建立会话、通知 UI 
	*/
	void HandleLoginCompleted(
		uint64 RequestGeneration,
		const FNiumaAccountLoginResult& Result);

	FNiumaAccountSessionLoginCompleted CancelPendingLogin();

	FNiumaAccountRuntimeSession RuntimeSession;

	FNiumaAccountSessionLoginCompleted ActiveLoginCompletion;

	bool bLoginPending = false;

	/**
	 * Logout 或 Deinitialize 后使旧 HTTP 回调失效。
	 */
	uint64 LoginRequestGeneration = 0;
};


