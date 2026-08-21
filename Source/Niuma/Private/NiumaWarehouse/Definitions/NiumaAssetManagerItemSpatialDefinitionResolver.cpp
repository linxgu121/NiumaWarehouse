#include "NiumaWarehouse/Definitions/NiumaAssetManagerItemSpatialDefinitionResolver.h"

#include "Engine/AssetManager.h"

#include "NiumaWarehouse/Definitions/NiumaItemDefinition.h"
#include "NiumaWarehouse/Spatial/Tool/NiumaItemFootprintUtility.h"

namespace 
{
	void SetResolverError(FString* OutError, const TCHAR* Message)
	{
		if (OutError != nullptr)
		{
			*OutError = Message;
		}
	}
}

const UNiumaItemDefinition* FNiumaAssetManagerItemSpatialDefinitionResolver::
FindOrLoadDefinition(
	const FPrimaryAssetId& ItemDefinitionId,
	FString* OutError) const 
{
    if (!ItemDefinitionId.IsValid())
    {
        SetResolverError(OutError,TEXT("待解析的 ItemDefinitionId 无效"));

        return nullptr;
    }

    if (ItemDefinitionId.PrimaryAssetType !=
        UNiumaItemDefinition::AssetType)
    {
        SetResolverError(OutError,TEXT("PrimaryAssetId 不是物品定义类型"));

        return nullptr;
    }

    UAssetManager& AssetManager = UAssetManager::Get();

    //GetPrimaryAssetObject 只查内存中的已加载对象，不触发磁盘 IO
    //如果资产已经被其他系统（如 UI 预加载、关卡初始化）加载过，零成本返回
    UNiumaItemDefinition* Definition =AssetManager.GetPrimaryAssetObject<UNiumaItemDefinition>(ItemDefinitionId);

    /*
     * GetPrimaryAssetObject 只查询内存。
     * 没有加载时，通过注册路径同步加载。
     */
    if (Definition == nullptr)
    {
        //存未命中时，通过 AssetManager 查注册表拿到 .uasset 的软引用路径
        const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ItemDefinitionId);

        if (!AssetPath.IsValid())
        {
            if (OutError != nullptr)
            {
                *OutError = FString::Printf(
                    TEXT("Asset Manager 中找不到物品定义：%s"),
                    *ItemDefinitionId.ToString());
            }

            return nullptr;
        }

        //TryLoad() 同步加载资产到内存
        UObject* LoadedObject = AssetPath.TryLoad();

        //阻塞调用，会卡当前线程直到文件读取完成
        Definition = Cast<UNiumaItemDefinition>(LoadedObject);
    }

    if (Definition == nullptr)
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("主资产不是 UNiumaItemDefinition：%s"),
                *ItemDefinitionId.ToString());
        }

        return nullptr;
    }

    //防止资产重命名/移动后 ID 漂移
    if (Definition->GetPrimaryAssetId() != ItemDefinitionId)
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("加载资产的 PrimaryAssetId 与请求不一致：%s"),
                *ItemDefinitionId.ToString());
        }

        return nullptr;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return Definition;
}

bool FNiumaAssetManagerItemSpatialDefinitionResolver::TryResolve(
    const FPrimaryAssetId& ItemDefinitionId,
    ENiumaItemOrientation Orientation,
    FNiumaResolvedItemSpatialData& OutData,
    FString* OutError) const
{
    FString DefinitionError;

    const UNiumaItemDefinition* Definition =
        FindOrLoadDefinition(
            ItemDefinitionId,
            &DefinitionError);

    if (Definition == nullptr)
    {
        if (OutError != nullptr)
        {
            *OutError = MoveTemp(DefinitionError);
        }

        return false;
    }

    FNiumaResolvedItemSpatialData CandidateData;

    CandidateData.ItemType = Definition->ItemType;

    CandidateData.RotationPolicy = Definition->RotationPolicy;

    FString FootprintError;

    if (!FNiumaItemFootprintUtility::TryRotate(
        Definition->Footprint,
        Orientation,
        CandidateData.Footprint,
        &FootprintError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("物品定义 Footprint 解析失败：%s"),
                *FootprintError);
        }

        return false;
    }

    FString ResolvedDataError;

    if (!CandidateData.IsStructurallyValid(
        &ResolvedDataError))
    {
        if (OutError != nullptr)
        {
            *OutError = FString::Printf(
                TEXT("解析后的物品空间数据无效：%s"),
                *ResolvedDataError);
        }

        return false;
    }

    // 全部成功后才修改正式输出。
    OutData = MoveTemp(CandidateData);

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}
