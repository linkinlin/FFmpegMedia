// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IMediaCache.h"
#include "IMediaPlayer.h"
#include "IMediaView.h"

#include "FFmpegMedia.h"
#include "FFmpegMediaTracks.h"

class IMediaEventSink;

/**
 * 实现UE播放器
 * IMediaPlayer Interface for media players.
 * IMediaCache Interface for access to a media player's cache.
 * IMediaView 	Interface for a media player's viewing settings.
 */
class FFmpegMediaPlayer
	: public IMediaPlayer
	, protected IMediaCache
	, protected IMediaView
{
public:
	FFmpegMediaPlayer(IMediaEventSink& InEventSink);
	~FFmpegMediaPlayer();
public:
	//IMediaPlayer接口
	virtual void Close() override;
	/**获取播放器缓存控件 */
	virtual IMediaCache& GetCache() override;
	/** 获取播放器的播放控件 */
	virtual IMediaControls& GetControls() override;
	/**
	* Get debug information about the player and currently opened media.
	* 获取播放器以及当前打开的媒体的媒体的调试信息, 触发事件EMediaEvent::MediaOpened和EMediaEvent::MediaOpenFailed调用 
	*/
	virtual FString GetInfo() const override;
	/**获取播放器插件的GUID */
	virtual FGuid GetPlayerPluginGUID() const override;
	/** 获取播放器的样本队列 */
	virtual IMediaSamples& GetSamples() override;
	/** 
	* Get playback statistics information.
	* 获取播放统计信息
	*/
	virtual FString GetStats() const override;
	/** 获取播放器轨道集合 */
	virtual IMediaTracks& GetTracks() override;
	/**获取当前加载的媒体的URL地址 */
	virtual FString GetUrl() const override;
	/**获取播放器视图配置 */
	virtual IMediaView& GetView() override;
	/**根据Url和可选参数打开媒体源 */
	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;
	virtual bool Open(const FString& Url, const IMediaOptions* Options, const FMediaPlayerOptions* PlayerOptions) override;
	/**根据Url和可选参数以及播放器参数打开媒体源 */
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* Options) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual bool FlushOnSeekStarted() const override;
	virtual bool FlushOnSeekCompleted() const override;
	/** 根据功能标识判断播放器功能是否支持 */
	virtual bool GetPlayerFeatureFlag(EFeatureFlag flag) const override;
protected:
	/**
	 * Initialize the native FFmpegMediaPlayer instance.
	 * [Custom] 初始化播放器
	 * @param Archive The archive being used as a media source (optional).
	 * @param Url The media URL being opened.
	 * @param Precache Whether to Precache media into RAM if InURL is a local file.
	 * @return true on success, false otherwise.
	 */
	bool InitializePlayer(const TSharedPtr<FArchive, ESPMode::ThreadSafe>& Archive, const FString& Url, bool Precache, const FMediaPlayerOptions* PlayerOptions);

private:
	/** [Custom] 读取媒体内容
	 * this thread gets the stream from the disk or the network
	 * 从文件或网络上获取流信息
	 * 参考ffplay static int read_thread(void *arg) 注意该方法并没有实现全部业务，只包含打开文件的那部分操作，其他操作则交给了FFmpegTracks类实现
	 */
	AVFormatContext* ReadContext(const TSharedPtr<FArchive, ESPMode::ThreadSafe>& Archive, const FString& Url, bool Precache);

	/** [Custom] 解码中断回调 Returns 1 when we would like to stop the application
	 * 参考ffplay static int decode_interrupt_cb(void* ctx)
	 * ffmpeg中常见阻塞函数
	 * avformat_open_input
	 * av_read_frame
	 * avformat_write_header
	 * av_write_trailer
	 * av_write_frame
	 * 返回 false:继续阻塞; true:退出阻塞
	 */
	static int decode_interrupt_cb(void* ctx);

	/** [Custom] 读取流回调 This is called when it's reading an Archive instead of an url*/
	static int ReadtStreamCallback(void* ptr, uint8_t* buf, int buf_size);

	/** [Custom] 跳转流回调 This is called when it's reading an Archive instead of an url*/
	static int64_t SeekStreamCallback(void* opaque, int64_t offset, int whence);

private:

	/** 媒体事件处理器 The media event handler. */
	IMediaEventSink& EventSink;

	/** 当前打开的媒体的地址.*/
	FString MediaUrl;

	/** 当前轨道集合 */
	TSharedPtr<FFFmpegMediaTracks, ESPMode::ThreadSafe> Tracks;

	/** 播放器是否停止，用于interrupt_callback.callback 
	* 注意该变量Player和Tracks中都存在，但在ffplay的VideoState中只有一个，
	* 关闭播放器时，本类中会设置为1，且会调用Tracks的Shutdown方法将Tracks中的abort_request设置为1
	*/
	int abort_request;

	/** 
	* FFmpeg 上下文
	*/
	AVFormatContext* ic;

	//当前Archive
	TSharedPtr<FArchive, ESPMode::ThreadSafe> CurrentArchive;
	//读取Archive时设置值
	AVIOContext* IOContext;
};
