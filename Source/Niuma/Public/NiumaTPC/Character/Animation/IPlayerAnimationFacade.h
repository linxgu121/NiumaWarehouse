#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "UObject/Interface.h"

#include "IPlayerAnimationFacade.generated.h"

class UAnimMontage;

/// 动画蒙太奇播放结束的回调委托
DECLARE_DELEGATE_OneParam(FNiumaMontageEndedCallback, bool);

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPlayerAnimationFacade : public UInterface
{
	GENERATED_BODY()
};


class NIUMA_API IPlayerAnimationFacade
{
	GENERATED_BODY()

public:

	/// <summary>
	/// 播放一次性动画蒙太奇
	/// </summary>
	/// <param name="Montage">要播放的动画蒙太奇</param>
	/// <param name="PlayRate">播放速率</param>
	/// <returns>动画播放时长</returns>
	virtual float PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f) = 0;

	/// <summary>
	/// 停止指定的动画蒙太奇
	/// </summary>
	/// <param name="Montage">要停止的动画蒙太奇</param>
	/// <param name="BlendOutTime">混合退出时间</param>
	virtual void StopMontage(UAnimMontage* Montage, float BlendOutTime = 0.2f) = 0;

	/// <summary>
	/// 指定的动画蒙太奇是否正在播放
	/// </summary>
	/// <param name="Montage">要检查的动画蒙太奇</param>
	/// <returns>如果动画蒙太奇正在播放则返回true，否则返回false</returns>
	virtual bool IsMontagePlaying(const UAnimMontage* Montage) const = 0;

	/// <summary>
	/// 获取指定动画蒙太奇的当前播放位置
	/// </summary>
	/// <param name="Montage">要获取播放位置的动画蒙太奇</param>
	/// <returns>动画蒙太奇的当前播放位置，单位为秒</returns>
	virtual float GetMontagePosition(const UAnimMontage* Montage) const = 0;

	/// <summary>
	/// 获取指定动画蒙太奇的归一化播放时间（0.0到1.0之间）
	/// </summary>
	virtual float GetMontageNormalizedTime(const UAnimMontage* Montage) const = 0;

	/// <summary>
	/// 注册"动画蒙太奇播放结束"时的回调通知
	/// 动画结束进行通知解决了"不知道动画什么时候结束"的异步问题，用于连击窗口、技能释放、换弹完成等时机敏感的游戏逻辑
	/// </summary>
	virtual void SetMontageEndedCallback(UAnimMontage* Montage, const FNiumaMontageEndedCallback& Callback) = 0;

	virtual void ClearMontageEndedCallback(UAnimMontage* Montage) = 0;
};
