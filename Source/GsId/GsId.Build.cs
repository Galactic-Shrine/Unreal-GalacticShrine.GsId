using System.IO;
using UnrealBuildTool;

public class GsId : ModuleRules
{
    public GsId(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bEnableExceptions = true;

        PublicDefinitions.Add("GALACTICSHRINE_GSID_API=GSID_API");

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        string ThirdPartyRoot = Path.GetFullPath(Path.Combine(
            ModuleDirectory,
            "..",
            "ThirdParty",
            "GalacticShrine",
            "GsId"
        ));
        string NativeHeader = Path.Combine(
            ThirdPartyRoot,
            "include",
            "galactic_shrine",
            "gsid",
            "gsid.hpp"
        );
        string NativeSource = Path.Combine(ThirdPartyRoot, "src", "gsid.cpp");

        if (!File.Exists(NativeHeader) || !File.Exists(NativeSource))
        {
            throw new BuildException(
                "GsId requires GalacticShrine.GsId.Cpp under "
                + "Source/ThirdParty/GalacticShrine/GsId. "
                + "The required native header or source file is missing."
            );
        }

        PublicIncludePaths.Add(Path.Combine(ThirdPartyRoot, "include"));
        ExternalDependencies.Add(NativeHeader);
        ExternalDependencies.Add(NativeSource);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("bcrypt.lib");
            PublicDefinitions.Add("GSID_PLATFORM_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicDefinitions.Add("GSID_PLATFORM_LINUX=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicFrameworks.Add("Security");
            PublicDefinitions.Add("GSID_PLATFORM_MACOS=1");
        }
        else
        {
            throw new BuildException(
                "GsId supports only Win64, Linux and macOS. "
                + "No secure random provider is configured for "
                + Target.Platform + "."
            );
        }
    }
}
