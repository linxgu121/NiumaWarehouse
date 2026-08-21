#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "NiumaTPC/Character/RuntimeData/Locomotion/PlayerLocomotionIntent.h"

#include "NiumaPlayerController.generated.h"

class APawn;
class UInputAction;
class UInputMappingContext;

struct FInputActionValue;

UCLASS()
class NIUMA_API ANiumaPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/// <summary>
	/// 控制器初始化阶段，仅执行 1 次，早于 `BeginPlay
	/// 绑定所有输入映射
	/// </summary>
	virtual void SetupInputComponent() override;

	/// <summary>
	/// 玩家生成、切换角色、传送重生都会调
	/// 角色切换后初始化：缓存角色指针、绑定动画实例、绑定角色委托、重置状态
	/// </summary>
	virtual void OnPossess(APawn* InPawn) override;
	/// <summary>
	/// 控制器与角色解绑：角色销毁、切换 Pawn、玩家离开场景
	/// **安全清空缓存指针、解绑委托**，防止野指针、悬空引用崩溃。
	/// </summary>
	virtual void OnUnPossess() override;

	/// <summary>
	/// 当前控制器销毁退出游戏时（关卡切换、游戏退出、对象被销毁）
	/// 销毁定时器、释放资源、存档、网络清理、彻底回收对象
	/// </summary>
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /// <summary>
    /// 可在蓝图编辑器面板配置，批量挂载多个输入上下文
    /// </summary>
    UPROPERTY(EditDefaultsOnly, Category = "Niuma|Input|Mappings")
    TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

    //// 三个核心输入动作
    UPROPERTY(EditDefaultsOnly, Category = "Niuma|Input|Actions")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Niuma|Input|Actions")
    TObjectPtr<UInputAction> WalkAction;

    UPROPERTY(EditDefaultsOnly, Category = "Niuma|Input|Actions")
    TObjectPtr<UInputAction> SprintAction;

    FPlayerLocomotionIntent LocomotionIntent;
    //防重复挂载上下文标记
    bool bMappingContextsInstalled = false;

    /// <summary>
    /// 将默认Mapping Context安装到本地玩家输入子系统。
    /// 由SetupInputComponent()调用。
    /// </summary>
    void InstallMappingContexts();

    /// <summary>
    /// 移除本Controller安装的Mapping Context。
    /// 由EndPlay()调用。
    /// </summary>
    void RemoveMappingContexts();

    /// <summary>
    /// 绑定注册
    /// </summary>
    void BindInputActions();

    /// <summary>
    /// 续读取二维移动向量，填充意图方向、速度
    /// </summary>
    void HandleMoveInput(const FInputActionValue& Value);

    /// <summary>
    /// 松开移动按键，清空移动方向
    /// </summary>
    void StopMoveInput();

    /// <summary>
    /// 慢走按键按下
    /// </summary>
    void StartWalkInput();
    /// <summary>
    /// 慢走按键抬起
    /// </summary>
    void StopWalkInput();

    /// <summary>
    /// 冲刺按键按下
    /// </summary>
    void StartSprintInput();
    /// <summary>
    /// 冲刺按键抬起
    /// </summary>
    void StopSprintInput();

    /// <summary>
    /// 把当前缓存的 LocomotionIntent 推送给所有实现接收接口的对象（动画蓝图、角色）
    /// </summary>
    void PublishLocomotionIntent();
    /// <summary>
    /// 清空所有移动状态，置零结构体，解绑 / 销毁时调用
    /// </summary>
    void ResetLocomotionIntent();
};