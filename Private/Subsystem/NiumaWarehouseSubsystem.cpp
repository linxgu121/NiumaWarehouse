#include "NiumaWarehouse/Subsystem/NiumaWarehouseSubsystem.h"

#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"

#include "NiumaWarehouse/Definitions/NiumaWarehouseDefinition.h"
#include "NiumaWarehouse/Settings/NiumaWarehouseSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogNiumaWarehouseSubsystem,Log,All);

namespace
{
    void SetInitializationError(
        FString* OutError,
        const FString& Message)
    {
        if (OutError != nullptr)
        {
            *OutError = Message;
        }
    }
}

void UNiumaWarehouseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializationError.Reset();

    if (!TryInitializeDefaultWarehouse(&InitializationError))
    {
        UE_LOG(
            LogNiumaWarehouseSubsystem,
            Error,
            TEXT("默认仓库初始化失败：%s"),
            *InitializationError);

        return;
    }

    const FNiumaSpatialContainerConfig& Config = Warehouse.GetConfig();

    UE_LOG(
        LogNiumaWarehouseSubsystem,
        Log,
        TEXT("默认仓库初始化成功：%d x %d"),
        Config.Width,
        Config.Height);
}

bool UNiumaWarehouseSubsystem::TryInitializeDefaultWarehouse(FString* OutError)
{
    const UNiumaWarehouseSettings* Settings = GetDefault<UNiumaWarehouseSettings>();

    if (Settings == nullptr)
    {
        SetInitializationError(
            OutError,
            TEXT("无法读取仓库项目设置"));

        return false;
    }

    const FSoftObjectPath DefinitionPath =
        Settings
        ->DefaultWarehouseDefinition
        .ToSoftObjectPath();

    if (!DefinitionPath.IsValid())
    {
        SetInitializationError(
            OutError,
            TEXT("没有配置默认仓库定义"));

        return false;
    }

    const UNiumaWarehouseDefinition* Definition =
        Settings
        ->DefaultWarehouseDefinition
        .LoadSynchronous();

    if (Definition == nullptr)
    {
        SetInitializationError(
            OutError,
            FString::Printf(
                TEXT("无法加载默认仓库定义：%s"),
                *DefinitionPath.ToString()));

        return false;
    }

    FNiumaSpatialContainerConfig ContainerConfig;
    FString ConfigError;

    if (!FNiumaSpatialContainerConfig::TryCreate(
        ENiumaItemContainerType::Warehouse,
        Definition->Width,
        Definition->Height,
        Definition->AcceptedItemTypes,
        ContainerConfig,
        &ConfigError))
    {
        SetInitializationError(
            OutError,
            FString::Printf(
                TEXT("默认仓库配置无效：%s"),
                *ConfigError));

        return false;
    }

    FString ContainerError;

    if (!Warehouse.TryInitializeEmpty(ContainerConfig,&ContainerError))
    {
        SetInitializationError(
            OutError,
            FString::Printf(
                TEXT("空仓库初始化失败：%s"),
                *ContainerError));

        return false;
    }

    if (OutError != nullptr)
    {
        OutError->Reset();
    }

    return true;
}

bool UNiumaWarehouseSubsystem::IsWarehouseInitialized() const
{
    return Warehouse.IsInitialized();
}

FString UNiumaWarehouseSubsystem::GetInitializationError() const
{
    return InitializationError;
}

const FNiumaSpatialContainerConfig& UNiumaWarehouseSubsystem::GetWarehouseConfig() const
{
    return Warehouse.GetConfig();
}

const FNiumaSpatialContainerState& UNiumaWarehouseSubsystem::GetWarehouseState() const
{
    return Warehouse.GetState();
}