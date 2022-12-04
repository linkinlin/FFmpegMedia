// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Event.h"
/**
 * 
 */
class FFmpegCond
{
public:
	FFmpegCond();
	~FFmpegCond();
public:
	/** 
	 * 发送信号量， 唤醒等待的线程
	 * 替代SDL_CondSignal
	 */
	void signal();

	/** 
	 * 释放锁，并等待信号量(signal)，接收到信号量之后，重新锁定，并返回，继续向下执行
	 * 替代SDL_CondWait
	 */
	int wait(FCriticalSection& mutex);

	/** 
	 * 释放锁，并等待信号量(signal)，并设置等待超时时间 
	 * 替代SDL_CondWaitTimeout
	 */
	int waitTimeout(FCriticalSection& mutex, unsigned int ms);
private:
	FEvent* event;
};
