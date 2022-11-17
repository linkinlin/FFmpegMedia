// Fill out your copyright notice in the Description page of Project Settings.


#include "FFmpegMediaSettings.h"

UFFmpegMediaSettings::UFFmpegMediaSettings()
	: SyncType(ESynchronizationType::AudioMaster)
	, UseHardwareAcceleratedCodecs(false)
	, UseInfiniteBuffer(false)
	, FrameDropStrategy(FrameDropStrategy::Default)
	, AudioVolume(100)
	, AllowFast(false)
	, DecoderReorderPtsStrategy(DecoderReorderPtsStrategy::Auto)
	, DisableAudio(false)
	, DisableVideo(false)
	, AudioThreadsCount(0)
	, VideoThreadsCount(0)
	, RtspTransport(ERtspTransport::Default)
{ }