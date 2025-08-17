// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WebPSupport : ModuleRules
{
	public WebPSupport(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			});
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			});
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"WebPSupportLibrary",
				"Projects"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"RenderCore"
			});
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}
