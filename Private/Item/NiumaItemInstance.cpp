#include "NiumaWarehouse/Item/NiumaItemInstance.h"

namespace
{
    void SetItemInstanceError(FString* OutError, const TCHAR* Message)
    {
        if (OutError != nullptr)
        {
            *OutError = Message;
        }
    }
}

/*
* 工厂创建
* 生成新的 FGuid 作为实例唯一 ID
* 封装输入参数（定义 ID、数量）
* 先构造候选对象（Candidate），验证通过后才原子性提交到 OutItemInstance
* 避免输出参数被"污染"（如果中途报错，OutItemInstance 不会被部分修改）
*/
bool FNiumaItemInstance::TryCreate(
    const FPrimaryAssetId& InItemDefinitionId,
    int32 InCount,
    FNiumaItemInstance& OutItemInstance,
    FString* OutError)
{
    FNiumaItemInstance Candidate;

    Candidate.InstanceId = FGuid::NewGuid();
    Candidate.ItemDefinitionId = InItemDefinitionId;
    Candidate.Count = InCount;

    if (!Candidate.IsValid(OutError))
    {
        return false;
    }

    // 所有规则通过后才提交正式输出。
    OutItemInstance = Candidate;

    return true;
}

/*
* 自验证
* IsValid 既被 TryCreate 用，也可独立调用
* 从存档反序列化后，可以直接 Instance.IsValid() 做脏数据清洗
*/
bool FNiumaItemInstance::IsValid(FString* OutError) const
{
    if (!InstanceId.IsValid())
    {
        SetItemInstanceError(OutError,TEXT("物品实例的 InstanceId 无效"));

        return false;
    }

    if (!ItemDefinitionId.IsValid())
    {
        SetItemInstanceError(OutError,TEXT("物品实例的 ItemDefinitionId 无效"));

        return false;
    }

    if (Count <= 0)
    {
        SetItemInstanceError(OutError,TEXT("物品实例的 Count 必须大于 0"));

        return false;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}
