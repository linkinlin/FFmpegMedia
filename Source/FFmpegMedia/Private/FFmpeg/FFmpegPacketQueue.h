// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "FFmpegCond.h"
#include <mutex>
extern "C" {
    #include "libavutil/fifo.h"
}

struct MyAVPacketList;
struct AVPacket;

/**
 * @brief 包队列
*/
class FFmpegPacketQueue
{

public:
	FFmpegPacketQueue();
	~FFmpegPacketQueue();

//public:
//
//    /**
//    * 放入一个Packet，大小不足，则会扩充
//    * 替换 static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
//    */
//    int PutPrivate(AVPacket* pkt);
//
//    /**
//    * 放入一个Packet，首先复制Packet到新Packet，然后调用PutPrivate添加
//    * 替换 static int packet_queue_put(PacketQueue* q, AVPacket* pkt);
//    */
//    int Put(AVPacket* pkt);
//
//    /**
//    * 放入一个空Packet，然后调用Put添加
//    * 替换  static int packet_queue_put_nullpacket(PacketQueue* q, AVPacket* pkt, int stream_index);
//    */
//    int PutNullpacket(AVPacket* pkt, int stream_index);
//
//    /**
//    * 初始化Packet队列，todo: 后续使用构造函数替换
//    * 替换  static int packet_queue_init(PacketQueue* q);
//    */
//    int Init();
//
//    /**
//    * 刷新队列，将队列中的数据释放掉，并初始化队列
//    * 替换   static void packet_queue_flush(PacketQueue* q);
//    */
//    void Flush();
//
//    /**
//    * 销毁Packet队列，todo: 后续使用析构函数替换
//    * 替换  static void packet_queue_destroy(PacketQueue* q);
//    */
//    void Destroy();
//
//    /**
//    * 中止队列
//    * 替换 static void packet_queue_abort(PacketQueue* q);
//    */
//    void Abort();
//
//    /**
//    * 启用队列
//    * 替换 static void packet_queue_start(PacketQueue* q);
//    */
//    void Start();
//
//    /**
//    * 从队列中取出一个Packet
//    * 替换  static int packet_queue_get(PacketQueue* q, AVPacket* pkt, int block, int* serial);
//    * return < 0 if aborted, 0 if no packet and > 0 if packet.
//    */
//    int Get(AVPacket* pkt, int block, int* serial);
//public:
//    int GetAbortRequest();
//    int GetSerial();
//    int GetNbPackets();

public:
    int packet_queue_put_private(AVPacket* pkt_);
    int packet_queue_put(AVPacket* pkt_);
    int packet_queue_put_nullpacket(AVPacket* pkt_, int stream_index_);
    int packet_queue_init();
    void packet_queue_flush();
    void packet_queue_destroy();
    void packet_queue_abort();
    void packet_queue_start();
    int packet_queue_get(AVPacket* pkt_, int block_, int* serial_);

public:
    AVFifo* pkt_list; //一个FIFO队列
    int nb_packets; //表示队列当前存储的数据包数量
    int size; //表示队列当前存储的包的总大小(字节为单位)
    int64_t duration; //表示队列当前存储的数据包的总时长(微秒为单位)
    int abort_request; //表示是否中止请求
    int serial; //表示队列序列号
    FCriticalSection* mutex;
    FFmpegCond* cond;
};
