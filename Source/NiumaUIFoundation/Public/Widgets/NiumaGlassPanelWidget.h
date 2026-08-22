#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "NiumaGlassPanelWidget.generated.h"

class UBackgroundBlur;
class UBorder;
class UNiumaUITheme;

/**
 * 全局玻璃面板原子组件
 * 玻璃面板基类
 *
 * 只处理面板自身的模糊、背景、边框和内容间距，
 * 不得访问仓库或其他业务 Subsystem。
 */
UCLASS(Abstract, Blueprintable)
class NIUMAUIFOUNDATION_API UNiumaGlassPanelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/*
	* 预览 + 运行都执行,属性初始化、应用 Theme 令牌、设置颜色圆角
	*/
	virtual void NativePreConstruct() override;

    /**
     * 负责模糊面板后方的画面。
     * 派生 Widget Blueprint 必须存在同名控件。
     */
    UPROPERTY(BlueprintReadOnly,meta = (BindWidget),Category = "Niuma|UI|GlassPanel")
    TObjectPtr<UBackgroundBlur> BackgroundBlur = nullptr;

    /**
     * 负责绘制半透明背景、圆角和边框。
     * 派生 Widget Blueprint 必须存在同名控件。
     */
    UPROPERTY(BlueprintReadOnly,meta = (BindWidget),Category = "Niuma|UI|GlassPanel")
    TObjectPtr<UBorder> PanelBorder = nullptr;

private:
    /**
     * 读取全局 Theme，并把设计令牌应用到控件。
     */
    void ApplyTheme();

    /**
     * 强引用已经加载的 Theme，防止使用过程中被回收。
     */
    UPROPERTY(Transient)
    TObjectPtr<UNiumaUITheme> ResolvedTheme = nullptr;
};