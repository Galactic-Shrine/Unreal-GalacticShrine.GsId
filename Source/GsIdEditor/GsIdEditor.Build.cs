using UnrealBuildTool;

public class GsIdEditor : ModuleRules
{
    public GsIdEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GsId",
            "UnrealEd"
        });
    }
}
