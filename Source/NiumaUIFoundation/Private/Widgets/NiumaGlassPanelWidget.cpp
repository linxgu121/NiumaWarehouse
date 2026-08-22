#include "Widgets/NiumaGlassPanelWidget.h"

#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Settings/NiumaUISettings.h"
#include "Styling/SlateBrush.h"
#include "Theme/NiumaUITheme.h"

//定义一个局部日志分类，专门给玻璃面板组件打印日志
DEFINE_LOG_CATEGORY_STATIC(LogNiumaGlassPanel,Log,All);

void UNiumaGlassPanelWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    ApplyTheme();
}

void UNiumaGlassPanelWidget::ApplyTheme()
{
    //获取UDeveloperSettings 的全局默认单例对象，拿到项目设置里配置的 UI 配置
    const UNiumaUISettings* Settings = GetDefault<UNiumaUISettings>();

    if(Settings == nullptr || Settings->DefaultTheme.IsNull())
    {
        UE_LOG(LogNiumaGlassPanel,Warning,TEXT("没有配置默认 Niuma UI Theme。"));
        return;
    }

    //同步加载这个 Theme 资产到内存(`DefaultTheme`是 `TSoftObjectPtr<UNiumaUITheme>软对象指针)
    ResolvedTheme = Settings->DefaultTheme.LoadSynchronous();

    if (ResolvedTheme == nullptr)
    {
        UE_LOG(LogNiumaGlassPanel,Error,TEXT("默认 Niuma UI Theme 加载失败。"));
        return;
    }

    if (BackgroundBlur == nullptr ||PanelBorder == nullptr)
    {
        UE_LOG(LogNiumaGlassPanel,Error,TEXT("玻璃面板缺少 BackgroundBlur 或 PanelBorder。"));
        return;
    }

    //让模糊效果跟随控件自身透明度 Alpha
    BackgroundBlur->SetApplyAlphaToBlur(true);

    //设置背景模糊强度
    BackgroundBlur->SetBlurStrength(ResolvedTheme->Shape.GlassBlurStrength);

    //`FSlateBrush`是 UMG 底层画笔，负责绘制图片、色块
    FSlateBrush PanelBrush;
    //`DrawAs = RoundedBox`：把画笔绘制模式设置为圆角方框，支持圆角，用于玻璃面板背景
    PanelBrush.DrawAs = ESlateBrushDrawType::RoundedBox;

    //给画笔设置面板底色，颜色取自 Theme 令牌里玻璃面板半透明颜色
    PanelBrush.TintColor = FSlateColor(ResolvedTheme->Colors.GlassPanel);

    //`FSlateBrushOutlineSettings(圆角,描边颜色,描边宽度)`
    //圆角方框的描边配置
    PanelBrush.OutlineSettings = FSlateBrushOutlineSettings(
            ResolvedTheme->Shape.CornerRadius,
            FSlateColor(
                ResolvedTheme->Colors.GlassPanelBorder),
            ResolvedTheme->Shape.BorderWidth);

    //把组装好的画笔完整交给`UBorder`控件，一次性应用底色、圆角、描边
    PanelBorder->SetBrush(PanelBrush);
    //设置面板内边距，面板内部内容距离边框的空白，现在使用`Medium`档位
    PanelBorder->SetPadding(FMargin(ResolvedTheme->Spacing.Medium));
}

