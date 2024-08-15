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
			"MaterialShaderQualitySettings",
			"RenderCore", // Essential for rendering operations
			"RHI", // Essential for low-level rendering interface

		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Renderer",
			"Projects",
			"TextureCompressor",
		});

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("UnrealEd"); // For editor-related functionality
		}
	}
}