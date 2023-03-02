// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class FFmpegMediaLibrary : ModuleRules
{
	public FFmpegMediaLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		bool isLibrarySupported = false;
		string prefixDir = "";
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			isLibrarySupported = true;
			prefixDir = "x64";
			// Add the import library
			string[] libs = { "avcodec.lib", "avdevice.lib", "avfilter.lib", "avformat.lib", "avutil.lib", "postproc.lib", "swresample.lib", "swscale.lib" };
			foreach (string lib in libs)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, prefixDir, "lib", lib));
			}

			// Delay-load the DLL, so we can load it from the right place first
			//PublicDelayLoadDLLs.Add("ExampleLibrary.dll");

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
		else if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{
			isLibrarySupported = true;
			prefixDir = "arm64";
			string[] libs = { "avcodec.lib", "avdevice.lib", "avfilter.lib", "avformat.lib", "avutil.lib", "postproc.lib", "swresample.lib", "swscale.lib" };
			foreach (string lib in libs)
			{
				PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, prefixDir, "lib", lib));
			}
			string[] dlls = { "avcodec-59.dll", "avdevice-59.dll", "avfilter-8.dll", "avformat-59.dll", "avutil-57.dll", "swresample-4.dll", "swscale-6.dll", "postproc-56.dll" };
			foreach (string dll in dlls)
			{
				PublicDelayLoadDLLs.Add(dll);
				RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/" + dll);
			}
			PublicDefinitions.Add("SCL_SECURE_NO_WARNINGS");
		}
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
			isLibrarySupported = true;
			/*PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/FFmpegMediaLibrary/Mac/Release/libExampleLibrary.dylib");*/

			string[] dlls = { "libavcodec.59.dylib", "libavdevice.59.dylib", "libavfilter.8.dylib", "libavformat.59.dylib", "libavutil.57.dylib", "libpostproc.56.dylib", "libswresample.4.dylib", "libswscale.6.dylib" };
			foreach (string dll in dlls)
			{
				string LinuxSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "FFmpegMediaLibrary", "Mac", dll);
				PublicDelayLoadDLLs.Add(LinuxSoPath);
				RuntimeDependencies.Add(LinuxSoPath);
			}
		}
        else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			isLibrarySupported = true;
			string[] dlls = { "libavcodec.so.59.37.100", "libavdevice.so.59.7.100", "libavfilter.so.8.44.100", "libavformat.so.59.27.100", "libavutil.so.57.28.100", "libpostproc.so.56.6.100", "libswresample.so.4.7.100", "libswscale.so.6.7.100" };
			foreach (string dll in dlls)
			{
				string LinuxSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "FFmpegMediaLibrary", "Linux", "x86_64", dll);
				PublicAdditionalLibraries.Add(LinuxSoPath);
				PublicDelayLoadDLLs.Add(LinuxSoPath);
				RuntimeDependencies.Add(LinuxSoPath);
			}
		}
		if (isLibrarySupported && prefixDir.Length > 0)
		{
			// Include path
			PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Path.Combine(ModuleDirectory, prefixDir, "include")
				}
			);
		}
	}
}
