// Copyright Evianaive. All Rights Reserved.

using UnrealBuildTool;

public class AnimCurveViewerDeveloper : ModuleRules
{
	public AnimCurveViewerDeveloper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"AnimGraph",
				"AnimCurveViewer",
				"KismetCompiler",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"BlueprintGraph",
					"ToolMenus",
					"Slate",
					"SlateCore",
					"GraphEditor",
					"EditorStyle"
				}
			);
		}
	}
}
