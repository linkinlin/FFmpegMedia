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
    AVPacket* pkt;
    FFmpegPacketQueue* queue;
    AVCodecContext* avctx;
    int pkt_serial;
    int finished;
    int packet_pending;
    FFmpegCond* empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
    FRunnableThread* decoder_tid;

    int decoder_reorder_pts = -1; //todo: 做成配置类
};