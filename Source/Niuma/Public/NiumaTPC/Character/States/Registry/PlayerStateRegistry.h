#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Templates/UniquePtr.h"

#include "NiumaTPC/Character/States/Types/PlayerStateType.h"

class FPlayerBaseState;

class FPlayerStateRegistry
{
public:
	FPlayerStateRegistry();
	~FPlayerStateRegistry();

	//把 拷贝构造、拷贝赋值、移动构造、移动赋值 全部禁用
	FPlayerStateRegistry(const FPlayerStateRegistry&) = delete;
	FPlayerStateRegistry& operator=(const FPlayerStateRegistry&) = delete;
	FPlayerStateRegistry(FPlayerStateRegistry&&) = delete;
	FPlayerStateRegistry& operator=(FPlayerStateRegistry&&) = delete;

	bool RegisterState(TUniquePtr<FPlayerBaseState> State);

	FPlayerBaseState* FindState(EPlayerStateType StateType);
	const FPlayerBaseState* FindState(EPlayerStateType StateType) const;

	/// <summary>
	/// 查询某状态是否已注册
	/// </summary>
	bool ContainsState(EPlayerStateType StateType) const;

	/// <summary>
	/// 返回已注册状态数量
	/// </summary>
	int32 Num() const;

	/// <summary>
	/// 清空所有已注册状态
	/// </summary>
	void Reset();

private:
	TMap<EPlayerStateType, TUniquePtr<FPlayerBaseState>> States;
};

