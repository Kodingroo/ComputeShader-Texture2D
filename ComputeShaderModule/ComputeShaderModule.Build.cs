using UnrealBuildTool;

public class ComputeShaderModule : ModuleRules
{
	public ComputeShaderModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PrivateIncludePaths.AddRange(new string[] 
		{
			"ComputeShaderModule/Private"
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"Engine",
			"MaterialShaderQualitySettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Renderer",
			"RenderCore",
			"RHI",
			"Projects"
		});

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("TargetPlatform");
		}
	}
}