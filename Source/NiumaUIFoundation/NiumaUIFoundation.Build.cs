using UnrealBuildTool;

public class NiumaUIFoundation : ModuleRules
{
    public NiumaUIFoundation(
        ReadOnlyTargetRules Target)
        : base(Target)
    {
        PCHUsage =
            PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "UMG",
                "DeveloperSettings"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore"
            });
    }
}