// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IMediaSamples.h"
#include "IMediaTracks.h"

#include "CoreTypes.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Internationalization/Text.h"
#include "IMediaSamples.h"
#include "IMediaTracks.h"
#include "IMediaControls.h"
#include "Math/IntPoint.h"
#include "Templates/SharedPointer.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "MediaPlayerOptions.h"
#include "FFmpegFrameQueue.h"
#include "FFmpegPacketQueue.h"
#include "FFmpegClock.h"
#include "FFmpegCond.h"
#include "LambdaFunctionRunnable.h"
#include "FFmpegDecoder.h"
#include "MediaSampleQueue.h"
#include "IMediaEventSink.h"

extern  "C" {
#include "libavformat/avformat.h"
#include "libavutil/macros.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavcodec/avfft.h"
}

struct AVFormatContext;

class FFFmpegMediaAudioSamplePool;
class FFFmpegMediaTextureSamplePool;

typedef struct AudioParams {
	int freq;
	AVChannelLayout ch_layout;
	enum AVSampleFormat fmt;
	int frame_size;
	int bytes_per_sec;
} AudioParams;

enum {
	AV_SYNC_AUDIO_MASTER, /* default choice */
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

/**
 * 
 */
class FFFmpegMediaTracks
	: public IMediaSamples
	, public IMediaTracks
	, public IMediaControls
{

	/**轨道格式 Track format. */
	struct FFormat
	{
		enum AVMediaType MediaType; //媒体类型，轨道中包含的信息，比如视频、音频、字母等等
		enum AVCodecID CodecID; //编码类型
		FString TypeName; //类型名称
		
		//音频结构
		struct AudioFormat
		{
			uint32 FrameSize; //
			uint32 NumChannels;
			uint32 SampleRate;
			AVChannelLayout ChannelLayout;
			enum AVSampleFormat Format;
			uint32 BytesPerSec;
			uint32 HardwareSize;
		}
		Audio;

		//视频结构
		struct VideoFormat
		{
			int64_t BitRate;
			float FrameRate;
			FIntPoint OutputDim;
			enum AVPixelFormat Format;
			int  LineSize[4];
		}
		Video;
	};

	/** 轨道信息 Track information. */
	struct FTrack
	{
		FText DisplayName;  //显示名称
		FFormat Format;     //轨道格式
		FString Language;   //语言
		FString Name;       //名称
		bool Protected;     //是否保护
		int StreamIndex;    //轨道索引
	};

public:
	FFFmpegMediaTracks();
	virtual ~FFFmpegMediaTracks();

public:
	/**
	 * 初始化
	 * Initialize the track collection.
	 * 
	 * @param IC 描述了一个媒体文件或媒体流的构成和基本信息
	 * @param Url 媒体地址
	 * @see PlayerOptions 播放选项
	 * 参考static VideoState *stream_open(const char *filename,const AVInputFormat *iformat)
	 */
	void Initialize(AVFormatContext* IC, const FString & Url, const FMediaPlayerOptions * PlayerOptions);

	/** 将ffmpeg流信息添加到轨道集合 */
	bool AddStreamToTracks(uint32 StreamIndex, const FMediaPlayerTrackOptions& TrackOptions, FString& OutInfo);

	/** 获取媒体信息 */
	const FString& GetMeidaInfo() const
	{
		return this->MediaInfo;
	}

	/**
	 * 清楚当前标识
	 *
	 * @see GetFlags
	 */
	void ClearFlags();

	/**
	 * 获取当前标识
	 *
	 * @param OutMediaSourceChanged Will indicate whether the media source changed.
	 * @param OutSelectionChanged Will indicate whether the track selection changed.
	 * @see ClearFlags
	 */
	void GetFlags(bool& OutMediaSourceChanged, bool& OutSelectionChanged) const;

	/** Thread to convert the video frames*/
	int DisplayThread();
	int AudioRenderThread();
	/** 获取媒体事件 */
	void GetEvents(TArray<EMediaEvent>& OutEvents);
	/** 关闭轨道 */
	void Shutdown();
	/** 音频渲染 参考sdl_audio_callback*/
	FTimespan RenderAudio();
	/** 转换废弃的编码 */
	AVPixelFormat ConvertDeprecatedFormat(AVPixelFormat format);
public:
	//~ IMediaSamples interface
	virtual bool FetchAudio(TRange<FTimespan> TimeRange, TSharedPtr<IMediaAudioSample, ESPMode::ThreadSafe>& OutSample) override;
	virtual bool FetchCaption(TRange<FTimespan> TimeRange, TSharedPtr<IMediaOverlaySample, ESPMode::ThreadSafe>& OutSample) override;
	virtual bool FetchMetadata(TRange<FTimespan> TimeRange, TSharedPtr<IMediaBinarySample, ESPMode::ThreadSafe>& OutSample) override;
	/** 丢弃所有未完成的媒体样本, 何时触发参考MediaPlayerFacade中的 */
	virtual void FlushSamples() override;
	virtual bool PeekVideoSampleTime(FMediaTimeStamp& TimeStamp) override;
	virtual EFetchBestSampleResult FetchBestVideoSampleForTimeRange(const TRange<FMediaTimeStamp>& TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample, bool bReverse) override;
	/** 当前实现无用，该方法会在EFeatureFlag::UsePlaybackTimingV2特性下调用 */
	//virtual bool FetchVideo(TRange<FTimespan> TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample) override;
	//virtual bool FetchVideo(TRange<FMediaTimeStamp> TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample) override;
public:
	//~ IMediaTracks interface
	/**
	 * 获取指定音频轨道的格式
	 */
	virtual bool GetAudioTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaAudioTrackFormat& OutFormat) const override;
	/** 
	* 获取指定轨道格式的数量(很重要，就Editor而言，返回0，界面上不会显示轨道)
	* ffmpeg中流类型 enum AVMediaType{  AVMEDIA_TYPE_UNKNOWN=-1, AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_AUDIO, AVMEDIA_TYPE_DATA,AVMEDIA_TYPE_SUBTITLE,AVMEDIA_TYPE_ATTACHMENT, AVMEDIA_TYPE_NB  
	* 故无法支持ue4中所有轨道类型，这里按照
	*      AVMEDIA_TYPE_VIDEO->Video
	*      AVMEDIA_TYPE_AUDIO->Audio
	*      AVMEDIA_TYPE_SUBTITLE->Caption
	* 支持者三种
	*/
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override;
	/**
	 * 	获取指定轨道的格式数量(很重要，就Editor而言，返回0，界面上不会显示格式)
	 *  依赖ffmpeg支持返回1或者0
	*/
	virtual int32 GetNumTrackFormats(EMediaTrackType TrackType, int32 TrackIndex) const override;
	/** 
	 * 获取指定轨道类型的当前选定轨道的索引
	 */
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	/** 
	* 获取指定轨道的人类可读名称 
	* 如果不为空，则界面上显示的轨道名称就是该返回值，否者显示GetTrackName的返回值
	* 当时实现总是返回空
	*/
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	/** 
	 * 获取指定轨道类型的当前所选格式的索引
	 */
	virtual int32 GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const override;
	/** 
	 * 获取指定轨道的语言 
	 */
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	/** 
	 * 获取指定轨道的内部名称 
	 */
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	/**
	 * 获取指定视频轨道的格式
	 */
	virtual bool GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaVideoTrackFormat& OutFormat) const override;
	/** 
	 * 选择指定的轨道(以Editor为例，)
	 */
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;
	/** 
	* 设置指定轨道上的活动格式
	* 目前来说，因为格式就一种，没法切换
	*/
	virtual bool SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex) override;
	/** 
	* 设置指定视频轨道的帧速率 
	*/
	virtual bool SetVideoTrackFrameRate(int32 TrackIndex, int32 FormatIndex, float FrameRate) override;

public:
	//~ IMediaControls interface
	 /** 判断当前播放器是否进行指定操作 */
	virtual bool CanControl(EMediaControl Control) const override;
	/** 获取播放总时长 */
	virtual FTimespan GetDuration() const override;
	/** 获取播放速率 */
	virtual float GetRate() const override;
	/** 获取播放器当前状态,十分重要, 会根据该操作决定界面显示，如果使用Editor的话 */
	virtual EMediaState GetState() const override;
	virtual EMediaStatus GetStatus() const override;
	/** 获取播放器支持帧率 */
	virtual TRangeSet<float> GetSupportedRates(EMediaRateThinning Thinning) const override;
	/** 获取当前播放时间(支持UsePlaybackTimingV2后，已经废弃，不会调用) */
	virtual FTimespan GetTime() const override;
	/** 是否循环播放*/
	virtual bool IsLooping() const override;
	/** Seek操作(重绕操作属于Seek操作)*/
	virtual bool Seek(const FTimespan& Time) override;
	/**设置是否循环播放*/
	virtual bool SetLooping(bool Looping) override;
	/**设置播放器速率 */
	virtual bool SetRate(float Rate) override;
//ffmpeg方法
private:
	/** 判断是否是实时流 */
	int is_realtime(AVFormatContext* s);
	/** 读取线程 */
	int read_thread();
	/** 判断是会否有足够的包 */
	int stream_has_enough_packets(AVStream* st, int stream_id, FFmpegPacketQueue* queue);
	/** 打开指定的流 */
	int stream_component_open(int stream_index);
	/** 关闭指定的流 */
	void stream_component_close(int stream_index);
	/** 音频解码线程 */
	int audio_thread();
	/** 视频解码线程 */
	int video_thread();
	/** 字幕解码线程 */
	int subtitle_thread();
	/**获取视频解码帧 */
	int get_video_frame(AVFrame* frame);
	/** 音频解码 */
	int audio_decode_frame(FTimespan& time, FTimespan& duration);
	/** 获取主同步类型 */
	int get_master_sync_type();
	/**获取音视频主同步锁 */
	double get_master_clock();
	/** 视频刷新 */
	void video_refresh(double* remaining_time);
	int queue_picture(AVFrame* src_frame, double pts, double duration, int64_t pos, int serial);
	void video_display();
	void video_image_display();
	int upload_texture(FFmpegFrame* vp, AVFrame* frame);
	/** 更新视频pts */
	void update_video_pts(double pts, int64_t pos, int serial);
	void step_to_next_frame();
	/* pause or resume the video */
	void stream_toggle_pause();
	/* seek in the stream */
	void stream_seek(int64_t pos, int64_t rel, int by_bytes);
	/** 计算时长 */
	double vp_duration(FFmpegFrame* vp, FFmpegFrame* nextvp);
	/** 计算延迟 */
	double compute_target_delay(double delay);
	/** 同步外部时钟 */
	void check_external_clock_speed();
	/** 音视频同步 */
	int synchronize_audio(int nb_samples);
private:

	/** 当前播放状态 Media playback state.  */
	EMediaState CurrentState;

	/**  同步写入轨道集合、选择集合、  Synchronizes write access to track arrays, selections & sinks. */
	mutable FCriticalSection CriticalSection;

	/** 媒体源是否改变 Whether the media source has changed. */
	bool MediaSourceChanged;

	/** 轨道是否改变 Whether the track selection changed. */
	bool SelectionChanged;

	/** 记录媒体信息字符串. */
	FString MediaInfo;
	/** 媒体选项 */
	FMediaPlayerTrackOptions MediaTrackOptions;


	/** 音频轨道列表 The available audio tracks. 初始化时填充值 */
	TArray<FTrack> AudioTracks;

	/** 视频轨道列表 The available video tracks.初始化时填充值 */
	TArray<FTrack> VideoTracks;

	/** 字幕轨道列表 The available caption tracks.初始化时填充值 */
	TArray<FTrack> CaptionTracks;

	/** The available metadata tracks. */
	TArray<FTrack> MetadataTracks;


	/** Index of the selected audio track. 当前选择的音频轨道 */
	int32 SelectedAudioTrack;

	/** Index of the selected caption track. 当前选择的字幕轨道 */
	int32 SelectedCaptionTrack;
	
	/** Index of the selected caption track. 当前选择的字幕轨道 */
	int32 SelectedMetadataTrack;

	/** Index of the selected video track. 当前选择的视频轨道*/
	int32 SelectedVideoTrack;

	FTimespan Duration; //总时长
	FTimespan CurrentTime;// 当前播放时间
	double CurrentRate;//当前播放速率

	bool ShouldLoop;//循环播放

	//视频是否播放中
	bool             displayRunning;
	FRunnableThread* displayThread;
	//音频是否播放中
	bool             audioRunning;
	FRunnableThread* audioRenderThread;

	TArray<uint8> ImgaeCopyDataBuffer; //图像拷贝数据缓存

	/** Audio sample object pool. */
	FFFmpegMediaAudioSamplePool* AudioSamplePool;
	TMediaSampleQueue<IMediaAudioSample> AudioSampleQueue;
	/** Overlay sample queue. */
	TMediaSampleQueue<IMediaOverlaySample> CaptionSampleQueue;
	/** Metadata sample queue. */
	TMediaSampleQueue<IMediaBinarySample> MetadataSampleQueue;
	/** Video sample object pool. */
	FFFmpegMediaTextureSamplePool* VideoSamplePool;
	/** Video sample queue. */
	TMediaSampleQueue<IMediaTextureSample> VideoSampleQueue;

	//定义一个媒体事件队列，通过TickInput将事件读取并发送
	/** Media events to be forwarded to main thread. */
	TQueue<EMediaEvent> DeferredEvents;

	FFormat::AudioFormat         audio_src; //源音频格式
	FFormat::AudioFormat         audio_tgt; //目标音频格式（当前好像总是一致）
	int currentOpenStreamNumber; //当前打开视频流数目，很重要，因为与ffplay中不同，UE中open stream和read是在两个线程中，需要保证所有流都开启之后，再读取
	int streamTotalNumber; //流总数 
	TOptional<FTimespan> SeekTimeOptional;

	double LastFetchVideoTime = 0; //最后视频包时间
//ffmpeg变量
private:
	AVFormatContext* ic; //上下文
	//AVCodecContext* video_avctx; //视频解码器codec上下文
	//AVCodecContext* audio_avctx; //音频解码器codec上下文
	//AVCodecContext* subtile_avctx; //字幕解码器codec上下文

	FFmpegFrameQueue pictq; //图片解码帧队列(picture)
	FFmpegFrameQueue subpq; //字幕解码帧队列(subtitle)
	FFmpegFrameQueue sampq; //音频解码帧队列(sampq)

	FFmpegPacketQueue audioq; //音频包队列
	FFmpegPacketQueue subtitleq; //字幕包队列
	FFmpegPacketQueue videoq; //视频包队列

	FFmpegClock audclk; //音频时钟
	FFmpegClock vidclk; //视频时钟
	FFmpegClock extclk; //外部时钟

	TSharedPtr<FFmpegDecoder> auddec; //音频解码器
	TSharedPtr<FFmpegDecoder> viddec; //视频解码器
	TSharedPtr<FFmpegDecoder> subdec; //字幕解码器

	double  max_frame_duration; //帧最大时长
	int realtime; //是否是实时流

	double audio_clock; // 音频时钟
	int audio_clock_serial; // 音频时钟序列
	int audio_volume; //音量

	int av_sync_type; //音视频同步类型
	FFmpegCond* continue_read_thread; //读线程锁，用于控制是否读取
	FRunnableThread* read_tid; //读取线程

	int abort_request; // 请求中断
	int paused; //停止
	int eof;  // 结束标志

	int queue_attachments_req; //队列附件请求, attached_pic的意思是有附带的图片。比如说一些MP3，AAC音频文件附带的专辑封面。所以，就是如果有，就去显示吧。

	double frame_timer; //当前已经播放的帧的开始显示时间
	int force_refresh; //画面强制刷新
	int frame_drops_late; //统计视频播放时丢弃的帧数量

	int step; //逐帧播放  默认为0，没有实现 废弃


	//seek 相关参数
	//int64_t start_time = AV_NOPTS_VALUE;
	int seek_req; //是否为seek请求
	int seek_flags;//seek标识
	int64_t seek_pos;//seek位置
	int64_t seek_rel; //seek增量(可为负值)
	double accurate_seek_time; //精准seek时间
	int accurate_audio_seek_flag; //精准音频seek标识
	int accurate_video_seek_flag; //精准视频seek标识
	int accurate_subtitle_seek_flag; //精准字幕seek标识



	int muted;
	int video_stream;	 //当前打开的视频流(索引)
	int subtitle_stream; //当前打开的字幕流(索引)
	int audio_stream;    //当前打开的音频流(索引)
	AVStream* video_st; //当前打开的视频流
	AVStream* audio_st; //当前打开的音频流
	AVStream* subtitle_st; //当前打开的字幕流
	int64_t audio_callback_time; //音频回调时间，

	int last_video_stream;
	int	last_audio_stream;
	int last_subtitle_stream;	

	double frame_last_filter_delay;
	int frame_drops_early;


	int audio_hw_buf_size; //与SDL相关
	uint8_t* audio_buf; //音频缓存
	unsigned int audio_buf_size; /* in bytes */ //音频缓存大小
	uint8_t* audio_buf1;
	unsigned int audio_buf1_size;
						 
	double audio_diff_cum; /* used for AV difference average computation */
	double audio_diff_avg_coef;
	double audio_diff_threshold;
	int audio_diff_avg_count;
	
	int last_paused;
	int read_pause_return;

	struct SwsContext* img_convert_ctx;
	struct SwrContext* swr_ctx;
	RDFTContext* rdft;
	int rdft_bits;
	FFTSample* rdft_data;

	/*FThreadSafeCounter VideoDropCounter;
	FThreadSafeCounter AudioDropCounter; */
	/*FThreadSafeCounter StopCounter;*/
	/*int testStop = 0;*/
};
