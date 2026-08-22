#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "NiumaUITheme.generated.h"

/**
 * 全局颜色设计令牌。
 */
USTRUCT(BlueprintType)
struct NIUMAUIFOUNDATION_API FNiumaUIColorTokens
{
    GENERATED_BODY()


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    /*
    * 页面底层背景色
    */
    FLinearColor PageBackground =
        FLinearColor(0.012f, 0.016f, 0.025f, 1.0f);

    /*
    * 玻璃态面板 / 弹窗 / 卡片背景
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor GlassPanel =
        FLinearColor(0.04f, 0.06f, 0.09f, 0.82f);

    /*
    * 主要文字
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor PrimaryText =
        FLinearColor(0.92f, 0.95f, 1.0f, 1.0f);

    /*
    * 次要文字
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor SecondaryText =
        FLinearColor(0.55f, 0.62f, 0.72f, 1.0f);

    /*
    * 错误提示文字颜色
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor ErrorText =
        FLinearColor(0.95f, 0.22f, 0.25f, 1.0f);

    /*
    * 强调色，高亮、选中、焦点
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor Accent =
        FLinearColor(0.0f, 0.78f, 1.0f, 1.0f);

    /*
    * 按钮普通状态颜色
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor ButtonNormal =
        FLinearColor(0.08f, 0.11f, 0.16f, 0.95f);

    /*
    * 鼠标悬浮状态
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor ButtonHovered =
        FLinearColor(0.04f, 0.32f, 0.42f, 1.0f);

    /*
    * 鼠标按下状态
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor ButtonPressed =
        FLinearColor(0.02f, 0.20f, 0.28f, 1.0f);

    /*
    * 按钮禁用置灰
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor ButtonDisabled =
        FLinearColor(0.10f, 0.11f, 0.13f, 0.55f);

    /**
    * 玻璃面板的边框颜色。
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Color")
    FLinearColor GlassPanelBorder =
        FLinearColor(0.0f, 0.78f, 1.0f, 0.28f);
};

/**
 * 全局字号设计令牌。
 *
 * 第一阶段先保存字号等级
 * 字体资产等真正使用时再加入。
 */
USTRUCT(BlueprintType)
struct NIUMAUIFOUNDATION_API FNiumaUITypographyTokens
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Typography", meta = (ClampMin = "1", UIMin = "1"))
    /*
    * 标题字号
    */
    int32 TitleSize = 32;

    /*
    * 正文字号
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Typography", meta = (ClampMin = "1", UIMin = "1"))
    int32 BodySize = 18;

    /*
    * 小字 / 备注字号
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Typography", meta = (ClampMin = "1", UIMin = "1"))
    int32 CaptionSize = 14;
};

/**
 * 全局间距设计令牌。
 */
USTRUCT(BlueprintType)
struct NIUMAUIFOUNDATION_API FNiumaUISpacingTokens
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Spacing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    /*
    * 小组件内部缝隙
    */
    float Small = 8.0f;

    /*
    * 常规组件间隔，最常用
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Spacing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float Medium = 16.0f;

    /*
    * 大板块之间的距离
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Spacing", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float Large = 24.0f;
};

/**
 * 全局面板形状与效果令牌。
 */
USTRUCT(BlueprintType)
struct NIUMAUIFOUNDATION_API FNiumaUIShapeTokens
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Shape", meta = (ClampMin = "0.0", UIMin = "0.0"))
    /*
    * 卡片、弹窗的圆角大小
    */
    float CornerRadius = 12.0f;

    /*
    * 面板边框描边粗细
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Shape", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BorderWidth = 1.0f;

    /*
    * 玻璃材质的模糊强度，数值越大背景越糊
    */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Effect", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float GlassBlurStrength = 12.0f;

};

/**
 * Niuma 全局 UI 设计令牌资产。
 *
 * 只保存视觉参数，不包含 Widget 行为，
 * 也不能访问任何业务 Subsystem。
 */
UCLASS(BlueprintType)
class NIUMAUIFOUNDATION_API UNiumaUITheme final : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Tokens")
    FNiumaUIColorTokens Colors;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Tokens")
    FNiumaUITypographyTokens Typography;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Tokens")
    FNiumaUISpacingTokens Spacing;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niuma|UI|Tokens")
    FNiumaUIShapeTokens Shape;


};



