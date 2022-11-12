// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
extern "C" {
    #include <libavcodec/avcodec.h>
}
#include "CoreMinimal.h"

/**
 * 定义FFmpeg帧
 * 用于保存一帧视频画面、音频或者字幕
 */
class FFmpegFrame
{
public:
	FFmpegFrame();
	~FFmpegFrame();
public:
    /** 初始化 */
    int Init();
    /** 获取Frame */
    AVFrame* GetFrame();
    int GetSerial();
    int64_t GetPos();
    double GetPts();
    void SetPts(double pts_);
    void SetPos(int64_t pos_);
    void SetSerial(int serial_);
    void SetDuration(double duration_);
    double GetDuration();
    void SetUploaded(int uploaded_);
    int GetUploaded();
    void SetSar(AVRational sar_);

    void SetWidth(int width_);
    void SetHeight(int height_);
    void SetFormat(int format_);

    AVSubtitle& GetSub();

public:
    /**
    * 替换static void frame_queue_unref_item(Frame* vp);
    */
    void UnrefItem();
public:
    AVFrame* frame; //视频或音频的解码数据
    AVSubtitle sub; //字幕的解码数据
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
};
