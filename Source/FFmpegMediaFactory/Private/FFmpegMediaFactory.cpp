// Copyright Epic Games, Inc. All Rights Reserved.

#include "FFmpegMediaFactory.h"
#include "Modules/ModuleManager.h"
//#include "Core.h"
#include "Interfaces/IPluginManager.h"
#include "IMediaModule.h"
#include "IMediaOptions.h"
#include "IMediaPlayerFactory.h"
#include "FFmpegMedia.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "FFmpegMediaSettings.h"
#endif

DEFINE_LOG_CATEGORY(LogFFmpegMediaFactory);

#define LOCTEXT_NAMESPACE "FFFmpegMediaFactoryModule"

class FFFmpegMediaFactoryModule 
	: public IMediaPlayerFactory
	, public IModuleInterface
{
public:

	/** Default constructor. */
	FFFmpegMediaFactoryModule() { }
public:
	//~ IMediaPlayerFactory interface 
	// IMediaPlayerFactory 接口实现

	/**
	 * 判断视频地址是否可以播放
	 * 注意该方法是需要手动调用的，比如在蓝图中播放视频时，调用该方法检测给定的视频地址是否可以播放
	 */
	virtual bool CanPlayUrl(const FString& Url, const IMediaOptions* Options, TArray<FText>* OutWarnings, TArray<FText>* OutErrors) const override {
		UE_LOG(LogFFmpegMediaFactory, Log, TEXT("FFmpegMediaFactory: CanPlayUrl %s"), *Url);
		FString Scheme;
		FString Location;

		// check scheme
		if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(LOCTEXT("NoSchemeFound", "No URI scheme found"));
			}

			return false;
		}

		if (!SupportedUriSchemes.Contains(Scheme))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(FText::Format(LOCTEXT("SchemeNotSupported", "The URI scheme '{0}' is not supported"), FText::FromString(Scheme)));
			}

			return false;
		}

		// check file extension
		if (Scheme == TEXT("file"))
		{
			const FString Extension = FPaths::GetExtension(Location, false);

			if (!SupportedFileExtensions.Contains(Extension))
			{
				if (OutErrors != nullptr)
				{
					OutErrors->Add(FText::Format(LOCTEXT("ExtensionNotSupported", "The file extension '{0}' is not supported"), FText::FromString(Extension)));
				}

				return false;
			}
		}

		// check options
		if ((OutWarnings != nullptr) && (Options != nullptr))
		{
			if (Options->GetMediaOption("PrecacheFile", false) && (Scheme != TEXT("file")))
			{
				OutWarnings->Add(LOCTEXT("PrecachingNotSupported", "Precaching is supported for local files only"));
			}
		}

		return true;
	}

	/**
	 * 创建一个播放器实例
	 */
	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) override {
		UE_LOG(LogFFmpegMediaFactory, Log, TEXT("FFmpegMediaFactory: CreatePlayer..."));

		auto FFmpegMediaModule = FModuleManager::LoadModulePtr<IFFmpegMediaModule>("FFmpegMedia");
		return (FFmpegMediaModule != nullptr) ? FFmpegMediaModule->CreatePlayer(EventSink) : nullptr;
	}

	/** 
	 * 获取播放器显示名称
	 */
	virtual FText GetDisplayName() const override {
		return LOCTEXT("MediaPlayerDisplayName", "FFmpeg");
	}

	/**
	 * 获取播放器名称
	 */
	virtual FName GetPlayerName() const override {
		static FName PlayerName(TEXT("FFmpegMedia"));
		return PlayerName;
	}

	/** 
	 * 获取播放器GUID
	 */
	virtual FGuid GetPlayerPluginGUID() const override {
		//todo: 重新生成一个
		static FGuid PlayerPluginGUID(0x688ae1e8, 0x9b647f80, 0x9ce98ced, 0x9daa4ca6);
		return PlayerPluginGUID;
	}

	/**
	 * 获取播放器支持的平台
	 */
	virtual const TArray<FString>& GetSupportedPlatforms() const override {
		return SupportedPlatforms;
	}

	/**
	 * 获取播放器支持的特性
	 */
	virtual bool SupportsFeature(EMediaFeature Feature) const override {

		return ((Feature == EMediaFeature::AudioSamples) ||
			(Feature == EMediaFeature::AudioTracks) ||
			(Feature == EMediaFeature::CaptionTracks) ||
			(Feature == EMediaFeature::MetadataTracks) ||
			(Feature == EMediaFeature::OverlaySamples) ||
			(Feature == EMediaFeature::SubtitleTracks) ||
			(Feature == EMediaFeature::VideoSamples) ||
			(Feature == EMediaFeature::VideoTracks));
	}


	/** IModuleInterface implementation */
	virtual void StartupModule() override {

		//根据模块名加载模块
		auto FFmpegMediaModule = FModuleManager::LoadModulePtr<IFFmpegMediaModule>("FFmpegMedia");

		// supported file extensions
		SupportedFileExtensions.Append(FFmpegMediaModule->GetSupportedFileExtensions());
		// supported schemes
		SupportedUriSchemes.Append(FFmpegMediaModule->GetSupportedUriSchemes());
		// supported platforms
		SupportedPlatforms.Add(TEXT("Windows"));
		SupportedPlatforms.Add(TEXT("HoloLens"));
		//todo: 支持linux
		//SupportedPlatforms.Add(TEXT("Mac"));
		//SupportedPlatforms.Add(TEXT("Android"));

		auto MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");
		if (MediaModule != nullptr)
		{
			MediaModule->RegisterPlayerFactory(*this);
			UE_LOG(LogFFmpegMediaFactory, Log, TEXT("Register FFmpegMediaFactory Success"));
		}
		else 
		{
			UE_LOG(LogFFmpegMediaFactory, Log, TEXT("MediaModule load fail, can not register FFmpegMediaFactory"));
		}

#if WITH_EDITOR
		// register settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "FFmpegMedia",
				LOCTEXT("FFmpegMediaSettingsName", "FFmpeg Media"),
				LOCTEXT("FFmpegMediaSettingsDescription", "Configure the FFmpeg Media plug-in."),
				GetMutableDefault<UFFmpegMediaSettings>()
			);
		}
#endif //WITH_EDITOR

	}
	virtual void ShutdownModule() override {
		// unregister player factory
		auto MediaModule = FModuleManager::GetModulePtr<IMediaModule>("Media");

		if (MediaModule != nullptr)
		{
			MediaModule->UnregisterPlayerFactory(*this);
		}

#if WITH_EDITOR
		// unregister settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "FFmpegMedia");
		}
#endif //WITH_EDITOR
	}

private:
	/** 支持的文件后缀类 */
	TArray<FString> SupportedFileExtensions;

	/** 支持的平台类型 */
	TArray<FString> SupportedPlatforms;

	/** 支持的uri类型. */
	TArray<FString> SupportedUriSchemes;
};


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFFmpegMediaFactoryModule, FFmpegMediaFactory)
