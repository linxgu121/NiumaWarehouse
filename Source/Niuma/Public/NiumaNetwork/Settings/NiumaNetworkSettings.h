#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "NiumaNetworkSettings.generated.h"

/**
 * Niuma 游戏后端网络配置。
 */
UCLASS(Config = Game,DefaultConfig,meta = (DisplayName = "Niuma Network"))
class NIUMA_API UNiumaNetworkSettings final : public UDeveloperSettings 
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	/**
	 * 游戏后端基础地址
	 *
	 * 本地开发允许 HTTP；
	 * Shipping 构建只允许 HTTPS。
	 */
	UPROPERTY(Config,EditAnywhere,Category = "Server")
	FString BaseUrl = TEXT("http://127.0.0.1:8809");

	/**
	 * 单次 HTTP 请求最大等待时间。
	 */
	UPROPERTY(Config,EditAnywhere,Category = "HTTP",
		meta = (
			ClampMin = "1.0",
			UIMin = "1.0",
			UIMax = "60.0"))
	float RequestTimeoutSeconds = 10.0f;


};

