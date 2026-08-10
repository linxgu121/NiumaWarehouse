#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouseOperationResult.generated.h"

/**
 * 仓库及二维空间容器操作结果。
 *
 * None 表示操作尚未执行或结果未赋值，
 * 避免默认构造时被错误地视为 Success。
 */
UENUM(BlueprintType)
enum class ENiumaWarehouseOperationResult : uint8
{
	None = 0 UMETA(DisplayName = "未产生结果"),

	Success = 10 UMETA(DisplayName = "成功"),

	NotInitialized = 20 UMETA(DisplayName = "容器未初始化"),

	InvalidItem = 30 UMETA(DisplayName = "物品实例无效"),

	InvalidCount = 40 UMETA(DisplayName = "物品数量无效"),

	InvalidPlacement = 50 UMETA(DisplayName = "放置记录无效"),

	MissingItemDefinition = 60 UMETA(DisplayName = "找不到物品定义"),

	InvalidItemDefinition = 70 UMETA(DisplayName = "物品定义无效"),

	UnsupportedItemType = 80 UMETA(DisplayName = "容器不接收该物品类型"),

	RotationNotAllowed = 90 UMETA(DisplayName = "物品不允许该旋转方向"),

	OutOfBounds = 100 UMETA(DisplayName = "超出容器边界"),

	Occupied = 110 UMETA(DisplayName = "目标格已被占用"),

	NoValidPlacement = 120 UMETA(DisplayName = "没有可用放置位置"),

	ItemNotFound = 130 UMETA(DisplayName = "容器中找不到该物品"),

	InternalError = 250 UMETA(DisplayName = "容器内部状态异常")
};
