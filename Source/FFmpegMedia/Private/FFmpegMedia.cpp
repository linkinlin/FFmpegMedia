// Copyright Epic Games, Inc. All Rights Reserved.

#include "FFmpegMedia.h"
#include "FFmpegMediaPlayer.h"

#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Templates/SharedPointer.h"

extern  "C" {
	#include "libavformat/avformat.h"
}

DEFINE_LOG_CATEGORY(LogFFmpegMedia);

#define LOCTEXT_NAMESPACE "FFFmpegMediaModule"

/**
 * Implements the FFmpegMedia module.
 */
class FFFmpegMediaModule : public IFFmpegMediaModule
{

public:

	/** Default constructor. */
	FFFmpegMediaModule()
		: Initialized(false)
	{ }

public:
	/**
	 * 创建播放器
	 */
	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) 
	{
		if (!Initialized)
		{
			UE_LOG(LogFFmpegMedia, Error, TEXT("FFmpegMediaModule not load, create FFmpegMediaPlayer failed"));
			return nullptr;
		}

		UE_LOG(LogFFmpegMedia, Log, TEXT("create FFmpegMediaPlayer success"));
		return MakeShareable(new FFmpegMediaPlayer(EventSink));
	}

	/**
	 * 获取平台支持的文件扩展名
	 */
	virtual TArray<FString> GetSupportedFileExtensions() 
	{
		TArray<FString> extensions;
		if (!Initialized)
		{
			return extensions;
		}

		TMap<FString, FString> extensionMap;
		void* oformat_opaque = NULL;
		const AVOutputFormat* oformat = av_muxer_iterate(&oformat_opaque);
		while (oformat) {
			FString ext = oformat->extensions;
			if (!ext.IsEmpty()) {
				TArray<FString> supportedExts;
				//分割成数组
				ext.ParseIntoArray(supportedExts, TEXT(","));
				for (const FString& s : supportedExts) {
					if (extensionMap.Contains(s)) {
						extensionMap[s] += TEXT(",") + FString(oformat->name);
					}
					else {
						extensionMap.Add(s, oformat->name);
					}
				}
			}
			oformat = av_muxer_iterate(&oformat_opaque);
		}

		extensionMap.GetKeys(extensions);
		return extensions;
	}

	/**
	 * 获取平台支持的URL
	 */
	virtual TArray<FString> GetSupportedUriSchemes() 
	{
		TArray<FString> protocols;
		if (!Initialized)
		{
			return protocols;
		}
		
		void* opaque = NULL;
		//0: 输入协议, 1: 输出协议， 这里获取输入协议
		const char* name = avio_enum_protocols(&opaque, 0);
		while (name) {
			protocols.Add(name);
			name = avio_enum_protocols(&opaque, 0);
		}
		return protocols;
	}

public:
	//~ IWmfMediaModule interface

	virtual void StartupModule() override 
	{
	    FString BaseDir = IPluginManager::Get().FindPlugin("FFmpegMedia")->GetBaseDir();
#if PLATFORM_WINDOWS || PLATFORM_HOLOLENS
#if PLATFORM_WINDOWS
		//开始动态加载ffmpeg dll文件
		FString avcodeLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/avcodec-60.dll"));
		FString avdeviceLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/avdevice-60.dll"));
		FString avfilterLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/avfilter-9.dll"));
		FString avformatLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/avformat-60.dll"));
		FString avutilLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/avutil-58.dll"));
		FString postprocLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/postproc-57.dll"));
		FString swresampleLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/swresample-4.dll"));
		FString swscaleLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/Win64/swscale-7.dll"));
#else
		FString avcodeLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/avcodec-59.dll"));
		FString avdeviceLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/avdevice-59.dll"));
		FString avfilterLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/avfilter-8.dll"));
		FString avformatLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/avformat-59.dll"));
		FString avutilLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/avutil-57.dll"));
		FString postprocLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/postproc-56.dll"));
		FString swresampleLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/swresample-4.dll"));
		FString swscaleLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/FFmpegMediaLibrary/HoloLens/swscale-6.dll"));
#endif
		AVUtilLibrary = !avutilLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*avutilLibraryPath) : nullptr;
		SWResampleLibrary = !swresampleLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*swresampleLibraryPath) : nullptr;
		PostProcLibrary = !postprocLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*postprocLibraryPath) : nullptr;
		SWScaleLibrary = !swscaleLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*swscaleLibraryPath) : nullptr;
		AVCodecLibrary = !avcodeLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*avcodeLibraryPath) : nullptr;
		AVFormatLibrary = !avformatLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*avformatLibraryPath) : nullptr;
		AVFilterLibrary = !avfilterLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*avfilterLibraryPath) : nullptr;
		AVDeviceLibrary = !avdeviceLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*avdeviceLibraryPath) : nullptr;
#endif	
		UE_LOG(LogFFmpegMedia, Display, TEXT("FFmpeg AVCodec version: %d.%d.%d"), LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO);
		UE_LOG(LogFFmpegMedia, Display, TEXT("FFmpeg license: %s"), UTF8_TO_TCHAR(avformat_license()));
		av_log_set_level(AV_LOG_INFO); //设置ffmpeg日志级别
		av_log_set_flags(AV_LOG_SKIP_REPEATED); //设置日志标识
		av_log_set_callback(&log_callback); //设置日志回调方法
		Initialized = true;
	}

	virtual void ShutdownModule() override 
	{
		if (!Initialized)
		{
			return;
		}

		if (AVDeviceLibrary) FPlatformProcess::FreeDllHandle(AVDeviceLibrary);
		if (AVFilterLibrary) FPlatformProcess::FreeDllHandle(AVFilterLibrary);
		if (PostProcLibrary) FPlatformProcess::FreeDllHandle(PostProcLibrary);
		if (SWScaleLibrary) FPlatformProcess::FreeDllHandle(SWScaleLibrary);
		if (AVFormatLibrary) FPlatformProcess::FreeDllHandle(AVFormatLibrary);
		if (AVCodecLibrary) FPlatformProcess::FreeDllHandle(AVCodecLibrary);
		if (SWResampleLibrary) FPlatformProcess::FreeDllHandle(SWResampleLibrary);
		if (AVUtilLibrary) FPlatformProcess::FreeDllHandle(AVUtilLibrary);
		Initialized = false;
	}

public:

	/** Virtual destructor. */
	virtual ~FFFmpegMediaModule() { }

public:
	/**
	 * @brief ffmpeg日志回调
	 * @param  
	 * @param level 
	 * @param format 
	 * @param arglist 
	*/
	static void  log_callback(void*, int level, const char* format, va_list arglist) 
	{

		char buffer[2048];
#if PLATFORM_WINDOWS
		vsprintf_s(buffer, 2048, format, arglist);
#else
		vsnprintf(buffer, 2048, format, arglist);
#endif
		FString str = TEXT("FFMPEG - ");
		str += buffer;

		switch (level) {
		case AV_LOG_TRACE:
			UE_LOG(LogFFmpegMedia, VeryVerbose, TEXT("%s"), *str);
			break;
		case AV_LOG_DEBUG:
			UE_LOG(LogFFmpegMedia, VeryVerbose, TEXT("%s"), *str);
			break;
		case AV_LOG_VERBOSE:
			UE_LOG(LogFFmpegMedia, Verbose, TEXT("%s"), *str);
			break;
		case AV_LOG_INFO:
			UE_LOG(LogFFmpegMedia, Display, TEXT("%s"), *str);
			break;
		case AV_LOG_WARNING:
			UE_LOG(LogFFmpegMedia, Warning, TEXT("%s"), *str);
			break;
		case AV_LOG_ERROR:
			UE_LOG(LogFFmpegMedia, Error, TEXT("%s"), *str);
			break;
		case AV_LOG_FATAL:
			UE_LOG(LogFFmpegMedia, Fatal, TEXT("%s"), *str);
			break;
		default:
			UE_LOG(LogFFmpegMedia, Display, TEXT("%s"), *str);
		}
	}

private:
	/** Whether the module has been initialized. */
	bool Initialized;

	void* AVUtilLibrary;
	void* SWResampleLibrary;
	void* AVCodecLibrary;
	void* SWScaleLibrary;
	void* AVFormatLibrary;
	void* PostProcLibrary;
	void* AVFilterLibrary;
	void* AVDeviceLibrary;
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFFmpegMediaModule, FFmpegMedia)
