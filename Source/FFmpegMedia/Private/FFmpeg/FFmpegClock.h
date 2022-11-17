// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FFmpegPacketQueue.h"
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0
/**
 * 
 */
class FFmpegClock
{
public:
	FFmpegClock();
	~FFmpegClock();
public:
    double Get();
    void SetAt(double pts, int serial, double time);
    void Set(double pts, int serial);
    void SetSpeed(double speed);
    void Init(FFmpegPacketQueue* queue);
    void Init(FFmpegClock* clock);
    void SyncToSlave(FFmpegClock* slave);
    int GetSerial();
    double GetLastUpdated();
    int GetPaused();
    void SetPaused(int paused_);
    double GetSpeed();
    double GetPts();
public:
    double pts;           /* clock base 时间基准*/
    double pts_drift;     /* clock base minus time at which we updated the clock 时间基减去更新时钟的时间*/
    double last_updated;
    double speed;
    int serial;           /* clock is based on a packet with this serial */ //播放序列
    int paused;
    int* queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection 队列的播放序列 PacketQueue中的 serial*/
};
