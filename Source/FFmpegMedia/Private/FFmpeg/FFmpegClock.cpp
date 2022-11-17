// Fill out your copyright notice in the Description page of Project Settings.


#include "FFmpeg/FFmpegClock.h"
extern  "C" {
#include "libavutil/time.h"
}


FFmpegClock::FFmpegClock()
{
    this->pts = 0.0;           /* clock base 时间基准*/
    this->pts_drift = 0.0;     /* clock base minus time at which we updated the clock 时间基减去更新时钟的时间*/
    this->last_updated = 0.0;
    this->speed = 0.0;
    this->serial = 0;          /* clock is based on a packet with this serial */ //播放序列
    this->paused = 0;
    this->queue_serial = nullptr;    /* pointer to the current packet queue serial, used for obsolete clock detection 队列的播放序列 PacketQueue中的 serial*/
}

FFmpegClock::~FFmpegClock()
{
}

double FFmpegClock::Get()
{
    if (*this->queue_serial != this->serial)
        return NAN;
    if (this->paused) {
        return this->pts;
    }
    else {
        double time = av_gettime_relative() / 1000000.0;
        return this->pts_drift + time - (time - this->last_updated) * (1.0 - this->speed);
    }
}

void FFmpegClock::SetAt(double pts_, int serial_, double time_)
{
    this->pts = pts_;
    this->last_updated = time_;
    this->pts_drift = this->pts - time_;
    this->serial = serial_;
}

void FFmpegClock::Set(double pts_, int serial_)
{
    double time = av_gettime_relative() / 1000000.0;
    this->SetAt(pts_, serial_, time);
}

void FFmpegClock::SetSpeed(double speed_)
{
    this->Set(this->Get(), this->serial);
    this->speed = speed_;
}

void FFmpegClock::Init(FFmpegPacketQueue* queue)
{
    this->speed = 1.0;
    this->paused = 0;
    this->queue_serial = &queue->serial;
    this->Set(NAN, -1);
}

void FFmpegClock::Init(FFmpegClock* clock)
{
    this->speed = 1.0;
    this->paused = 0;
    this->queue_serial = &clock->serial;
    this->Set(NAN, -1);
}

void FFmpegClock::SyncToSlave(FFmpegClock* slave)
{
    double clock = this->Get();
    double slave_clock = slave->Get();
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        this->Set(slave_clock, slave->serial);
}

int FFmpegClock::GetSerial()
{
    return this->serial;
}

double FFmpegClock::GetLastUpdated()
{
    return this->last_updated;
}

int FFmpegClock::GetPaused()
{
    return this->paused;
}

void FFmpegClock::SetPaused(int paused_)
{
    this->paused = paused_;
}

double FFmpegClock::GetSpeed()
{
    return this->speed;
}

double FFmpegClock::GetPts()
{
    return this->pts;
}
