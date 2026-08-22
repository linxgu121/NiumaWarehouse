#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "NiumaUISettings.generated.h"

class UNiumaUITheme;

/**
 * Project Settings(项目设置)中的 Niuma UI 配置入口。
 */
UCLASS(Config = Game,DefaultConfig, meta = (DisplayName = "Niuma UI"))
class NIUMAUIFOUNDATION_API UNiumaUISettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	//控制在项目设置面板的层级
	//大分类
	virtual FName GetCategoryName() const override;
	//大分类的子项
	virtual FName GetSectionName() const override;

    /**
     * 全局默认 Theme。
     *
     * 使用软引用，避免项目启动时因为配置对象
     * 自动同步加载全部 UI 资产。
     */
    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Theme")
    TSoftObjectPtr<UNiumaUITheme> DefaultTheme;
};