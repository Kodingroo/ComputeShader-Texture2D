using UnrealBuildTool;

public class ComputeShaderModuleEditor : ModuleRules
{
	public ComputeShaderModuleEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"Projects",
				"Blutility" // Blutility is required for the Blueprint Function Library
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"UMGEditor",
			"ComputeShaderModule"
		});

		DynamicallyLoadedModuleNames.AddRange(new string[] { });
		
		// For editor functionality, add this dependency
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("EditorStyle");
		}
	}
}