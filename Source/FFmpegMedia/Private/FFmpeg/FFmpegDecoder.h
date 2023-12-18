// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "FFmpegPacketQueue.h"
#include "FFmpegFrameQueue.h"
#include "FFmpegCond.h"
extern "C" {
    #include <libavcodec/avcodec.h>
}

/**
 * Decoder类
 */
class FFmpegDecoder
{

public:
    FFmpegDecoder();
    ~FFmpegDecoder();

//public:
//    /**
//    * 初始化Decoder
//    * 替换 static int decoder_init(Decoder* d, AVCodecContext* avctx, PacketQueue* queue, SDL_cond* empty_queue_cond) 
//    */
//    int Init(AVCodecContext* avctx, FFmpegPacketQueue* queue, FFmpegCond* empty_queue_cond);
//
//    /**
//    * 解码帧
//    * static int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub)
//    */
//    int DecodeFrame(AVFrame* frame, AVSubtitle* sub);
//
//    void SetStartPts(int64_t start_pts_);
//    void SetStartPtsTb(AVRational start_pts_tb_);
//    //int  Start(FRunnable* f2runnable, void* arg);
//    //int Start(std::function<int(void*)> thread_func, void* arg);
//    int Start(FString threadName, std::function<void()> f);
//    int GetPktSerial();
//    AVCodecContext* GetAvctx();
//    int GetFinished();
//
//    void Abort(FFmpegFrameQueue* fq);
//    void Destroy();


public:

    int decoder_start(FString threadName, std::function<void()> f);
    /**
     * @brief 解码器初始化
     * @param avctx 
     * @param queue 
     * @param empty_queue_cond 
     * @return 
    */
    int decoder_init(AVCodecContext* avctx_, FFmpegPacketQueue* queue_, FFmpegCond* empty_queue_cond_);

    /**
     * @brief 解码帧
     * @param frame 
     * @param sub 
     * @return 
    */
    int decoder_decode_frame(AVFrame* frame, AVSubtitle* sub);

    /**
     * @brief 中断
     * @param fq 
    */
    void decoder_abort(FFmpegFrameQueue* fq);

    /**
     * @brief 销毁
    */
    void decoder_destroy();

public:
    AVPacket* pkt; //当前解码器正在处理的包
    FFmpegPacketQueue* queue; //用于解码器解码的数据包存储队列
    AVCodecContext* avctx; //编码器上下文
    int pkt_serial; //表示解码器当前正在处理的数据包的序列号
    int finished;   //
    int packet_pending; //包解码是否存在异常状态
    FFmpegCond* empty_queue_cond; //
    int64_t start_pts; //解码器起始时间戳
    AVRational start_pts_tb; //解码器起始时间戳的时间基
    int64_t next_pts; //解码器下一时间戳
    AVRational next_pts_tb;  //解码器下一时间戳的时间基
    FRunnableThread* decoder_tid; //解码线程

    int decoder_reorder_pts = -1; //todo: 做成配置类
};