// Fill out your copyright notice in the Description page of Project Settings.


#include "FFmpeg/FFmpegDecoder.h"
#include "LambdaFunctionRunnable.h"

FFmpegDecoder::FFmpegDecoder()
{
}

FFmpegDecoder::~FFmpegDecoder()
{
}

/** 初始化解码器 */
int FFmpegDecoder::Init(AVCodecContext* avctx_, FFmpegPacketQueue* queue_, FFmpegCond* empty_queue_cond_)
{
    this->pkt = av_packet_alloc();
    if (!this->pkt)
        return AVERROR(ENOMEM);
    this->avctx = avctx_;
    this->queue = queue_;
    this->empty_queue_cond = empty_queue_cond_;
    this->start_pts = AV_NOPTS_VALUE;
    this->pkt_serial = -1;
    this->decoder_thread = NULL;
    return 0;
}

int FFmpegDecoder::DecodeFrame(AVFrame* frame, AVSubtitle* sub)
{
    //解码时重新排序 0=off 1=on -1=auto
    int decoder_reorder_pts = -1;
    int ret = AVERROR(EAGAIN);
    for (;;) {
        if (this->queue->GetSerial() == this->pkt_serial) {
            do {
                if (this->queue->GetAbortRequest())
                    return -1;

                switch (this->avctx->codec_type) {
                case AVMEDIA_TYPE_VIDEO:
                    ret = avcodec_receive_frame(this->avctx, frame);
                    if (ret >= 0) {
                        if (decoder_reorder_pts == -1) {
                            frame->pts = frame->best_effort_timestamp;
                        }
                        else if (!decoder_reorder_pts) {
                            frame->pts = frame->pkt_dts;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    ret = avcodec_receive_frame(this->avctx, frame);
                    if (ret >= 0) {
                        AVRational tb = { 1, frame->sample_rate };
                        if (frame->pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(frame->pts, this->avctx->pkt_timebase, tb);
                        else if (this->next_pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(this->next_pts, this->next_pts_tb, tb);
                        if (frame->pts != AV_NOPTS_VALUE) {
                            this->next_pts = frame->pts + frame->nb_samples;
                            this->next_pts_tb = tb;
                        }
                    }
                    break;
                }
                if (ret == AVERROR_EOF) {
                    this->finished = this->pkt_serial;
                    avcodec_flush_buffers(this->avctx);
                    return 0;
                }
                if (ret >= 0)
                    return 1;
            } while (ret != AVERROR(EAGAIN));
        }

        do {
            if (this->queue->GetNbPackets() == 0)
                this->empty_queue_cond->signal();
            if (this->packet_pending) {
                this->packet_pending = 0;
            }
            else {
                int old_serial = this->pkt_serial;
                if (this->queue->Get(this->pkt, 1, &this->pkt_serial) < 0)
                    return -1;
                if (old_serial != this->pkt_serial) {
                    avcodec_flush_buffers(this->avctx);
                    this->finished = 0;
                    this->next_pts = this->start_pts;
                    this->next_pts_tb = this->start_pts_tb;
                }
            }
            if (this->queue->GetSerial() == this->pkt_serial)
                break;
            av_packet_unref(this->pkt);
        } while (1);

        if (this->avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            int got_frame = 0;
            ret = avcodec_decode_subtitle2(this->avctx, sub, &got_frame, this->pkt);
            if (ret < 0) {
                ret = AVERROR(EAGAIN);
            }
            else {
                if (got_frame && !this->pkt->data) {
                    this->packet_pending = 1;
                }
                ret = got_frame ? 0 : (this->pkt->data ? AVERROR(EAGAIN) : AVERROR_EOF);
            }
            av_packet_unref(this->pkt);
        }
        else {
            if (avcodec_send_packet(this->avctx, this->pkt) == AVERROR(EAGAIN)) {
                av_log(this->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                this->packet_pending = 1;
            }
            else {
                av_packet_unref(this->pkt);
            }
        }
    }
}

void FFmpegDecoder::SetStartPts(int64_t start_pts_)
{
    this->start_pts = start_pts_;
}

void FFmpegDecoder::SetStartPtsTb(AVRational start_pts_tb_)
{
     this->start_pts_tb = start_pts_tb_;
}

int FFmpegDecoder::Start(FString threadName, std::function<void()> f)
{
    queue->Start();
    decoder_thread = LambdaFunctionRunnable::RunThreaded(threadName, f);
    if (!decoder_thread) {
        //av_log(NULL, AV_LOG_ERROR, "SDL_CreateThread(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    };
    return 0;
}

int FFmpegDecoder::GetPktSerial()
{
    return this->pkt_serial;
}

AVCodecContext* FFmpegDecoder::GetAvctx()
{
    return this->avctx;
}

int FFmpegDecoder::GetFinished()
{
    return this->finished;
}

void FFmpegDecoder::Abort(FFmpegFrameQueue* fq)
{
    this->queue->Abort();
    fq->Signal();

    if (this->decoder_thread) {
        this->decoder_thread->WaitForCompletion();
        this->decoder_thread = NULL;
    }
    this->queue->Flush();
}

void FFmpegDecoder::Destroy()
{
    av_packet_free(&this->pkt);
    avcodec_free_context(&this->avctx);
}


   
