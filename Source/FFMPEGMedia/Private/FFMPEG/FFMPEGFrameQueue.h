// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FFmpegFrame.h"
#include "FFmpegCond.h"
#include "FFmpegPacketQueue.h"
extern "C" {
    #include "libavutil/fifo.h"
}

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

/**
 * 帧队列
 * 
 */
class FFmpegFrameQueue
{
public:
	FFmpegFrameQueue();
	~FFmpegFrameQueue();
public:
    /**
    * 初始化队列
    * 
    * 替换 static int frame_queue_init(FrameQueue* f, PacketQueue* pktq, int max_size, int keep_last);
    */
    int Init(FFmpegPacketQueue* pktq, int max_size, int keep_last);
    /**
    * 销毁队列
    * 替换  static void frame_queue_destory(FrameQueue* f);
    */
    void Destory();
    /**
    * 替换 static void frame_queue_signal(FrameQueue* f);
    */
    void Signal();
    /**
    * 获取当前节点
    * 替换 static Frame* frame_queue_peek(FrameQueue* f);
    */
    FFmpegFrame* Peek();
    /**
    * 获取下一个节点
    * 替换 static Frame* frame_queue_peek_next(FrameQueue* f);
    */
    FFmpegFrame* PeekNext();
    /**
    * 获取上一个节点
    * 替换  static Frame* frame_queue_peek_last(FrameQueue* f);
    */
    FFmpegFrame* PeekLast();
    /**
    * 获取一个可写节点
    * 替换  static Frame* frame_queue_peek_writable(FrameQueue* f);
    */
    FFmpegFrame* PeekWritable();
    /**
    * 获取一个可读节点
    * 替换  static Frame* frame_queue_peek_readable(FrameQueue* f);
    */
    FFmpegFrame* PeekReadable();
    /**
    * 替换  static void frame_queue_push(FrameQueue* f);
    */
    void Push();
    /**
    * 替换 static void frame_queue_next(FrameQueue* f);
    */
    void Next();
    /**
    * 替换 static int frame_queue_nb_remaining(FrameQueue* f);
    */
    /* return the number of undisplayed frames in the queue */
    int NbRemaining();
    /**
    * 替换  static int64_t frame_queue_last_pos(FrameQueue* f);
    */
    /* return last shown position */
    int64_t LastPos();
    FCriticalSection* GetMutex();
    int GetRindexShown();
public:
    FFmpegFrame* queue[FRAME_QUEUE_SIZE]; //队列元素
    int rindex; //读指针
    int windex; //写指针
    int size;  //当前存储的节点个数
    int max_size; //最大允许存储的节点个数
    int keep_last; //是否要保留最后一个读节点
    int rindex_shown; //当前显示的节点
    FCriticalSection* mutex;
    FFmpegCond* cond;
    FFmpegPacketQueue* pktq; //关联的Packet队列
};
