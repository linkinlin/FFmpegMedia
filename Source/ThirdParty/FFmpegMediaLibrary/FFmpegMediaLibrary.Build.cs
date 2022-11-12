// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class FFmpegMediaLibrary : ModuleRules
{
	public FFmpegMediaLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Path.Combine(ModuleDirectory, "x64", "include")
				}
			);

			// Add the import library
			string[] libs = { "avcodec.lib", "avdevice.lib", "avfilter.lib", "avformat.lib", "avutil.lib", "postproc.lib", "swresample.lib", "swscale.lib" };
			foreach (string lib in libs)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "lib", lib));
			}

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("ExampleLibrary.dll");

			// Ensure that the DLL is staged along with the executable
			string[] dlls = { "avcodec-59.dll", "avdevice-59.dll", "avfilter-8.dll", "avformat-59.dll", "avutil-57.dll", "swresample-4.dll", "swscale-6.dll", "postproc-56.dll" };
			foreach (string dll in dlls)
			{
				PublicDelayLoadDLLs.Add(dll);
				RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/FFmpegMediaLibrary/Win64/" + dll);

				//System.Console.WriteLine("Will cp FFmpeg dll form " + sourceDLL + " to " + targetDLL);
				//RuntimeDependencies.Add(targetDLL, sourceDLL); //编译时
				//RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", dll), sourceDLL); //打包时
			}

		}
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/FFmpegMediaLibrary/Mac/Release/libExampleLibrary.dylib");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "FFmpegMediaLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);
		}
	}
}
