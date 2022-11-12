// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class FFmpegMedia : ModuleRules
{
	public FFmpegMedia(ReadOnlyTargetRules Target) : base(Target)
	{
		bEnableExceptions = true;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
					"Media",
			});

		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"FFmpegMedia/Private",
				"FFmpegMedia/Private/Player",
				"FFmpegMedia/Private/FFmpeg",
			}
			);


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
				"FFmpegMediaLibrary",
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"MediaUtils",
				"RenderCore",
				"Projects",
				"FFmpegMediaFactory"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				"Media",
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
