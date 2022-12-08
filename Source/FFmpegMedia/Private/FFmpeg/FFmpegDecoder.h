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
 * 
 */
class FFmpegDecoder
{
public:
    FFmpegDecoder();
    ~FFmpegDecoder();
public:
    /**
    * 初始化Decoder
    * 替换 static int decoder_init(Decoder* d, AVCodecContext* avctx, PacketQueue* queue, SDL_cond* empty_queue_cond) 
    */
    int Init(AVCodecContext* avctx, FFmpegPacketQueue* queue, FFmpegCond* empty_queue_cond);

    /**
    * 解码帧
    * static int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub)
    */
    int DecodeFrame(AVFrame* frame, AVSubtitle* sub);

    void SetStartPts(int64_t start_pts_);
    void SetStartPtsTb(AVRational start_pts_tb_);
    //int  Start(FRunnable* f2runnable, void* arg);
    //int Start(std::function<int(void*)> thread_func, void* arg);
    int Start(FString threadName, std::function<void()> f);
    int GetPktSerial();
    AVCodecContext* GetAvctx();
    int GetFinished();

    void Abort(FFmpegFrameQueue* fq);
    void Destroy();
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
    FRunnableThread* decoder_thread;
};