#include "NiumaWarehouse/Spatial/Tool/NiumaItemFootprintUtility.h"

namespace
{
	void SetFootprintError(FString* OutError, const TCHAR* Message)
	{
		if (OutError != nullptr)
		{
			*OutError = Message;
		}
	}

	void SortCells(TArray<FIntPoint>& Cells)
	{
		Cells.Sort([](const FIntPoint& Left, const FIntPoint& Right)
			{
				if (Left.Y != Right.Y)
				{
					return Left.Y < Right.Y;
				}

				return Left.X < Right.X;

			});
	}
}

bool FNiumaItemFootprintUtility::TryNormalize(
	const FNiumaItemFootprint& InFootprint,
	FNiumaItemFootprint& OutFootprint,
	FString* OutError)
{
	if (InFootprint.Cells.IsEmpty())
	{
		SetFootprintError(OutError, TEXT("物品至少需要一个占用格"));

		return false; 
	}

	//从 Footprint 的 Cells 数组中取第一个单元格坐标，作为后续计算最小边界（Minimum Bounds）的初始参考点
	FIntPoint Minimum = InFootprint.Cells[0];

	//存储唯一 2D 网格坐标的集合，用于自动去重和快速查找
	//UE 的哈希集合容器特性是：元素唯一O(1) 查找：判断某个坐标是否存在非常快无序存储：不保证遍历顺序
	TSet<FIntPoint> UniqueCells;
	//预先分配哈希桶内存
	UniqueCells.Reserve(InFootprint.Cells.Num());

	for(const FIntPoint& Cell : InFootprint.Cells)
	{
		if (Cell.X < 0 || Cell.Y < 0)
		{
			SetFootprintError(OutError, TEXT("物品空间不能包含负坐标"));

			return false;
		}

		if (UniqueCells.Contains(Cell))
		{
			SetFootprintError(OutError, TEXT("物品空间不能包含重复坐标"));

			return false;
		}

		//插入数据
		UniqueCells.Add(Cell);

		Minimum.X = FMath::Min(Minimum.X, Cell.X);
		Minimum.Y = FMath::Min(Minimum.Y, Cell.Y);


	}

	//存储经过“归一化”（标准化）处理后的网格坐标
	TArray<FIntPoint> NormalizedCells;
	NormalizedCells.Reserve(InFootprint.Cells.Num());

    for(const FIntPoint& Cell : InFootprint.Cells)
	{
		NormalizedCells.Add(Cell - Minimum);
	}

	SortCells(NormalizedCells);

	// 所有步骤成功后才替换正式输出。
	OutFootprint.Cells = MoveTemp(NormalizedCells);

	if (OutError != nullptr)
	{
		OutError->Reset();
	}

	return true;
	}

bool FNiumaItemFootprintUtility::TryRotate(
	const FNiumaItemFootprint& InFootprint,
	ENiumaItemOrientation Orientation,
	FNiumaItemFootprint& OutFootprint,
	FString* OutError)
{
	FNiumaItemFootprint NormalizedFootprint;

	//如果归一化处理失败则返回
	if (!TryNormalize(InFootprint,NormalizedFootprint,OutError))
	{
		return false;
	}

	int32 MaximumX = 0;
	int32 MaximumY = 0;

	//遍历找出 最大的 X 和 Y
	for (const FIntPoint& Cell : NormalizedFootprint.Cells)
	{
		MaximumX = FMath::Max(MaximumX, Cell.X);
		MaximumY = FMath::Max(MaximumY, Cell.Y);
	}

	//存放旋转后的新坐标
	TArray<FIntPoint> RotatedCells;
	RotatedCells.Reserve(NormalizedFootprint.Cells.Num());

	for (const FIntPoint& Cell : NormalizedFootprint.Cells)
	{
		//初始化
		//用于存放当前格子经过旋转变换后的新坐标
		FIntPoint RotatedCell = FIntPoint::ZeroValue;

		switch (Orientation)
		{
		case ENiumaItemOrientation::Degree0:
			RotatedCell = Cell;
			break;

		case ENiumaItemOrientation::Degree90:
			RotatedCell = FIntPoint(MaximumY - Cell.Y, Cell.X);
			break;

		case ENiumaItemOrientation::Degree180:
			RotatedCell = FIntPoint(MaximumX - Cell.X, MaximumY - Cell.Y);
			break;

		case ENiumaItemOrientation::Degree270:
			RotatedCell = FIntPoint(Cell.Y, MaximumX - Cell.X);
			break;

		default:
			SetFootprintError(OutError,TEXT("无效的物品旋转方向"));
			return false;
		}

		//存储旋转后的坐标
		RotatedCells.Add(RotatedCell);
	}

	//保证旋转结果使用统一的“先 Y 后 X”顺序
	SortCells(RotatedCells);

	// 全部成功后才修改正式输出
	OutFootprint.Cells = MoveTemp(RotatedCells);

	if (OutError != nullptr)
	{
		OutError->Reset();
	}

	return true;
}

