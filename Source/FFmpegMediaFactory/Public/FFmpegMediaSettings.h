// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FFmpegMediaSettings.generated.h"


UENUM()
enum class ESynchronizationType : uint8 {
	AudioMaster = 0, //音频
	VideoMaster,      //视频
	ExternalClock  //外部时钟
};


UENUM()
enum class FrameDropStrategy : int8 {
	Default = -1, //默认值，
	Allow = 1,      //允许丢帧
	NotAllow = 0  //不允许丢帧
};

UENUM()
enum class DecoderReorderPtsStrategy : int8 {
	Auto = -1, //自动(默认值)，
	On = 1,    //开启
	Off = 0  //关闭
};


UENUM()
enum class ERtspTransport : uint8 {
	Default = 0,
	Udp,
	UdpMulticast,
	Tcp,
	Http,
	Https
};


/**
 *  Settings for the FFmpegMedia plug-in.
 */
UCLASS(config = Engine, defaultconfig)
class FFMPEGMEDIAFACTORY_API UFFmpegMediaSettings : public UObject
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UFFmpegMediaSettings();

public:

	UPROPERTY(config, EditAnywhere, Category = Media, meta = (ToolTip = "是否使用硬件解码器"))
	bool UseHardwareAcceleratedCodecs;

	UPROPERTY(config, EditAnywhere, Category = Media, meta = (ToolTip = "非标准化规范的多媒体兼容优化"))
	bool AllowFast;



	//UPROPERTY(config, EditAnywhere, Category = Media)
	//ESynchronizationType SyncType; //同步类型

	////DisplayName = "UseInfiniteBuffer",
	//UPROPERTY(config, EditAnywhere, Category = Media, meta = (ToolTip = "don't limit the input buffer size (useful with realtime streams)"))
	//bool UseInfiniteBuffer; //是否限制缓存大小

	//UPROPERTY(config, EditAnywhere, Category = Media, meta = (ToolTip = "drop frames when cpu is too slow"))
	//FrameDropStrategy FrameDropStrategy; //丢帧策略

	//UPROPERTY(config, EditAnywhere, Category = Media, meta = (UIMin = 0, UIMax = 100))
	//uint8 AudioVolume; //音量(0-100)
	//
	//DecoderReorderPtsStrategy  DecoderReorderPtsStrategy; //Pts排序策略

	//UPROPERTY(config, EditAnywhere, Category = Media)
	//bool DisableAudio; //关闭音频

	//UPROPERTY(config, EditAnywhere, Category = Media)
	//bool DisableVideo; //关闭音频

	//UPROPERTY(config, EditAnywhere, Category = Media, meta = (UIMin = 0, UIMax = 16))
	//uint16 AudioThreadsCount;//音频解码线程数

	//UPROPERTY(config, EditAnywhere, Category = Media, meta = (UIMin = 0, UIMax = 16))
	//uint16 VideoThreadsCount; //视频解码线程数

	//UPROPERTY(config, EditAnywhere, Category = Media)
	//ERtspTransport RtspTransport; //rtsp协议
};
