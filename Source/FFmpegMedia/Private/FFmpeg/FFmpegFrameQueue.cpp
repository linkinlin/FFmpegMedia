// Fill out your copyright notice in the Description page of Project Settings.


#include "FFmpeg/FFmpegFrameQueue.h"

FFmpegFrameQueue::FFmpegFrameQueue()
{
    for (int i = 0; i < FRAME_QUEUE_SIZE; i++) {
        queue[i] = new FFmpegFrame();
    }
    rindex = 0;
    windex = 0;
    size = 0;
    max_size = 0;
    keep_last = 0;
    rindex_shown = 0;
    pktq = NULL;
}

FFmpegFrameQueue::~FFmpegFrameQueue()
{
}

int FFmpegFrameQueue::Init(FFmpegPacketQueue* pktq_, int max_size_, int keep_last_)
{
    int i;
    this->mutex = new FCriticalSection();
    if (!this->mutex) {
        av_log(NULL, AV_LOG_FATAL, "FFmpegFrameQueue CreateMutex Fail\n");
        return AVERROR(ENOMEM);
    }

    this->cond = new FFmpegCond();
    if (!this->cond) {
        av_log(NULL, AV_LOG_FATAL, "FFmpegFrameQueue CreateCond Fail\n");
        return AVERROR(ENOMEM);
    }

    this->pktq = pktq_;
    this->max_size = FFMIN(max_size_, FRAME_QUEUE_SIZE);
    this->keep_last = !!keep_last_;
    for (i = 0; i < this->max_size; i++)
        if (!(this->queue[i]->Init()))
            return AVERROR(ENOMEM);
    return 0;
}

void FFmpegFrameQueue::Destory()
{
    int i;
    for (i = 0; i < this->max_size; i++) {
        FFmpegFrame* vp = this->queue[i];
        if (vp != nullptr) {
            vp->UnrefItem();
            AVFrame* frame = vp->GetFrame();
            av_frame_free(&frame);
        }
    }
    //与ffplay不同, 因为会重复利用，所以此处尽可能重置所有字段
   /* rindex = 0;
    windex = 0;
    size = 0;
    max_size = 0;
    keep_last = 0;
    rindex_shown = 0;
    pktq = NULL;*/
    //SDL_DestroyMutex(f->mutex);
    //SDL_DestroyCond(f->cond);
}

void FFmpegFrameQueue::Signal()
{
    this->mutex->Lock();
    this->cond->signal();
    this->mutex->Unlock();
}

FFmpegFrame* FFmpegFrameQueue::Peek()
{
    return this->queue[(this->rindex + this->rindex_shown) % this->max_size];
}

FFmpegFrame* FFmpegFrameQueue::PeekNext()
{
    return this->queue[(this->rindex + this->rindex_shown + 1) % this->max_size];
}

FFmpegFrame* FFmpegFrameQueue::PeekLast()
{
    return this->queue[this->rindex];
}

FFmpegFrame* FFmpegFrameQueue::PeekWritable()
{
    /* wait until we have space to put a new frame */
    this->mutex->Lock();
    while (this->size >= this->max_size &&
        !this->pktq->GetAbortRequest()) {
        this->cond->wait(*this->mutex);
    }
    this->mutex->Unlock();
    if (this->pktq->GetAbortRequest())
        return NULL;

    return this->queue[this->windex];
}

FFmpegFrame* FFmpegFrameQueue::PeekReadable()
{
    /* wait until we have a readable a new frame */
    this->mutex->Lock();
    while (this->size - this->rindex_shown <= 0 &&
        !this->pktq->GetAbortRequest()) {
        this->cond->wait(*this->mutex);
    }
    this->mutex->Unlock();
    if (this->pktq->GetAbortRequest())
        return NULL;

    return this->queue[(this->rindex + this->rindex_shown) % this->max_size];
}

void FFmpegFrameQueue::Push()
{
    if (++this->windex == this->max_size)
        this->windex = 0;
    this->mutex->Lock();
    this->size++;
    this->cond->signal();
    this->mutex->Unlock();
}

void FFmpegFrameQueue::Next()
{
    if (this->keep_last && !this->rindex_shown) {
        this->rindex_shown = 1;
        return;
    }
    this->queue[this->rindex]->UnrefItem();
    if (++this->rindex == this->max_size)
        this->rindex = 0;
    this->mutex->Lock();
    this->size--;
    this->cond->signal();
    this->mutex->Unlock();
}
/** 判断是否剩余 */
int FFmpegFrameQueue::NbRemaining()
{
    return this->size - this->rindex_shown;
}

int64_t FFmpegFrameQueue::LastPos()
{
    FFmpegFrame* fp = this->queue[this->rindex];
    if (this->rindex_shown && fp->GetSerial() == this->pktq->GetSerial())
        return fp->GetPos();
    else
        return -1;
}

FCriticalSection* FFmpegFrameQueue::GetMutex()
{
    return this->mutex;
}

int FFmpegFrameQueue::GetRindexShown()
{
    return this->rindex_shown;
}

int FFmpegFrameQueue::frame_queue_init(FFmpegPacketQueue* pktq_, int max_size_, int keep_last_)
{
    int i;
    if (!(this->mutex = new FCriticalSection())) {
        av_log(NULL, AV_LOG_FATAL, "FFmpegFrameQueue CreateMutex Fail\n");
        return AVERROR(ENOMEM);
    }
    if (!(this->cond = new FFmpegCond())) {
        av_log(NULL, AV_LOG_FATAL, "FFmpegFrameQueue CreateCond Fail\n");
        return AVERROR(ENOMEM);
    }
    this->pktq = pktq_;
    this->max_size = FFMIN(max_size_, FRAME_QUEUE_SIZE);
    this->keep_last = !!keep_last_;
    for (i = 0; i < this->max_size; i++)
        if (!(this->queue[i]->frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    return 0;
}

void FFmpegFrameQueue::frame_queue_destroy()
{
    int i;
    for (i = 0; i < this->max_size; i++) {
        FFmpegFrame* vp = this->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    //SDL_DestroyMutex(f->mutex);
    //SDL_DestroyCond(f->cond);
}

void FFmpegFrameQueue::frame_queue_signal()
{
    this->mutex->Lock();
    this->cond->Signal();
    this->mutex->Unlock();
}

void FFmpegFrameQueue::frame_queue_unref_item(FFmpegFrame* vp)
{
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

FFmpegFrame* FFmpegFrameQueue::frame_queue_peek()
{
    return this->queue[(this->rindex + this->rindex_shown) % this->max_size];
}

FFmpegFrame* FFmpegFrameQueue::frame_queue_peek_next()
{
    return this->queue[(this->rindex + this->rindex_shown + 1) % this->max_size];
}

FFmpegFrame* FFmpegFrameQueue::frame_queue_peek_last()
{
    return this->queue[this->rindex];
}

FFmpegFrame* FFmpegFrameQueue::frame_queue_peek_writable()
{
    /* wait until we have space to put a new frame */
    this->mutex->Lock();
    while (this->size >= this->max_size &&
        !this->pktq->abort_request) {
        this->cond->Wait(this->mutex);
    }
    this->mutex->Unlock();

    if (this->pktq->abort_request)
        return NULL;

    return this->queue[this->windex];
}

FFmpegFrame* FFmpegFrameQueue::frame_queue_peek_readable()
{

    /* wait until we have a readable a new frame */
    this->mutex->Lock();
    while (this->size - this->rindex_shown <= 0 &&
        !this->pktq->abort_request) {
        this->cond->Wait(this->mutex);
    }
    this->mutex->Unlock();

    if (this->pktq->abort_request)
        return NULL;

    return this->queue[(this->rindex + this->rindex_shown) % this->max_size];
}

void FFmpegFrameQueue::frame_queue_push()
{
    if (++this->windex == this->max_size)
        this->windex = 0;
    this->mutex->Lock();
    this->size++;
    this->cond->Signal();
    this->mutex->Unlock();
}

void FFmpegFrameQueue::frame_queue_next()
{

    if (this->keep_last && !this->rindex_shown) {
        this->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(this->queue[this->rindex]);
    if (++this->rindex == this->max_size)
        this->rindex = 0;
    this->mutex->Lock();
    this->size--;
    this->cond->Signal();
    this->mutex->Unlock();
}

/**
 * @brief 
 * @return return the number of undisplayed frames in the queue 
*/
int FFmpegFrameQueue::frame_queue_nb_remaining()
{
    return this->size - this->rindex_shown;
}

/**
 * @brief return last shown position
 * @return 
*/
int64_t FFmpegFrameQueue::frame_queue_last_pos()
{
    FFmpegFrame* fp = this->queue[this->rindex];
    if (this->rindex_shown && fp->serial == this->pktq->serial)
        return fp->pos;
    else
        return -1;
}
