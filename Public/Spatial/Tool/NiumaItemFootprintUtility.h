#pragma once

#include "CoreMinimal.h"

#include "NiumaWarehouse/Spatial/NiumaItemFootprint.h"
#include "NiumaWarehouse/Type/NiumaItemOrientation.h"

/**
 * Footprint(占用空间) 的纯 C++ 校验与转换工具
 */
struct NIUMA_API FNiumaItemFootprintUtility final
{
	/**
	 * 校验输入形状，并将最小坐标平移到 (0,0)。
	 */
	static bool TryNormalize(
	  const FNiumaItemFootprint& InFootprint,
	  FNiumaItemFootprint& OutFootprint,
	  FString* OutError = nullptr);

	/**
     * 相对于物品原始形状，生成指定方向的规范化 Footprint。
     */
	static bool TryRotate(
		const FNiumaItemFootprint& InFootprint,
		ENiumaItemOrientation Orientation,
		FNiumaItemFootprint& OutFootprint,
		FString* OutError = nullptr);
};
