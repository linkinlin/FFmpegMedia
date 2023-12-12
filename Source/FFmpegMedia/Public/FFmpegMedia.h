// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "IMediaModule.h"
#include "IMediaOptions.h"
#include "IMediaPlayerFactory.h"
#include "Logging/LogMacros.h"

/** Log category for the FFmpegMedia module. */
DECLARE_LOG_CATEGORY_EXTERN(LogFFmpegMedia, Verbose, All);

/**
 * Interface for the FFmpegMedia module.
 */
class IFFmpegMediaModule
	: public IModuleInterface
{
public:
	
	/**
	 * @brief 创建播放器实例
	 * @param EventSink 
	 * @return 
	*/
	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) = 0;

	/**
	 * @brief 获取播放器支持的文件后缀列表
	 * @return 
	*/
	virtual TArray<FString> GetSupportedFileExtensions() = 0;

	/**
	 * @brief 获取播放器支持的uri类型
	 * @return 
	*/
	virtual TArray<FString> GetSupportedUriSchemes() = 0;

public:

	/** Virtual destructor. */
	virtual ~IFFmpegMediaModule() { }
};


