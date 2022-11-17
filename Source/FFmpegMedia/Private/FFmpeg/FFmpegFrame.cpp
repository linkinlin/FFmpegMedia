// Fill out your copyright notice in the Description page of Project Settings.


#include "FFmpeg/FFmpegFrame.h"

FFmpegFrame::FFmpegFrame()
{
    frame = NULL;
    sub = { 0 };
    serial = 0;
    pts = 0.0;
    duration = 0.0;
    pos = 0;
    width = 0;
    height = 0;
    format = 0;
    uploaded = false;
    flip_v = false;
}

FFmpegFrame::~FFmpegFrame()
{
    av_frame_unref(frame);
    avsubtitle_free(&sub);
}

int FFmpegFrame::Init()
{
    frame = av_frame_alloc();
    return frame == NULL ? 0 : 1;
}

AVFrame* FFmpegFrame::GetFrame()
{
    return this->frame;
}

int FFmpegFrame::GetSerial()
{
    return this->serial;
}

int64_t FFmpegFrame::GetPos()
{
    return this->pos;
}

double FFmpegFrame::GetPts()
{
    return this->pts;
}

void FFmpegFrame::SetPts(double pts_)
{
    this->pts = pts_;
}

void FFmpegFrame::SetPos(int64_t pos_)
{
    this->pos = pos_;
}

void FFmpegFrame::SetSerial(int serial_)
{
    this->serial = serial_;
}

void FFmpegFrame::SetDuration(double duration_)
{
    this->duration = duration_;
}

double FFmpegFrame::GetDuration()
{
    return this->duration;
}

void FFmpegFrame::SetUploaded(int uploaded_)
{
    this->uploaded = uploaded_;
}

int FFmpegFrame::GetUploaded()
{
    return this->uploaded;
}

void FFmpegFrame::SetSar(AVRational sar_)
{
    this->sar = sar_;
}

void FFmpegFrame::SetWidth(int width_)
{
    this->width = width_;
}

void FFmpegFrame::SetHeight(int height_)
{
    this->height = height_;
}

void FFmpegFrame::SetFormat(int format_)
{
    this->format = format_;
}

AVSubtitle& FFmpegFrame::GetSub()
{
    return this->sub;
}

void FFmpegFrame::UnrefItem()
{
    av_frame_unref(this->frame); //frame计数减1
    avsubtitle_free(&this->sub); //sub关联的内存释放
}
