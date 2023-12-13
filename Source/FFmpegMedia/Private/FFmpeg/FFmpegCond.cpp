// Fill out your copyright notice in the Description page of Project Settings.


#include "FFmpeg/FFmpegCond.h"
#include "FFmpegCond.h"
//#include "GenericPlatform/GenericPlatformProcess.h"

#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"

FFmpegCond::FFmpegCond()
{
	event = FGenericPlatformProcess::GetSynchEventFromPool();
	//event = FPlatformProcess::GetSynchEventFromPool(true);
}

FFmpegCond::~FFmpegCond()
{
	/*if (event) {
		FGenericPlatformProcess::ReturnSynchEventToPool(event);
	}*/
	//event->Trigger();
	FPlatformProcess::ReturnSynchEventToPool(event);
}

void FFmpegCond::signal()
{
	if (event) {
		event->Trigger();
	}
}

int FFmpegCond::wait(FCriticalSection& mutex)
{
	return waitTimeout(mutex, 0);
}

int FFmpegCond::waitTimeout(FCriticalSection& mutex, unsigned int ms)
{
	mutex.Unlock();
	if (ms == 0) {
		event->Wait();
		mutex.Lock();
		return 0;
	}
	else
	{
		bool wait_result = event->Wait(FTimespan::FromMicroseconds(ms));
		mutex.Lock();
		if (!wait_result) {
			return 1;
		}
		return 0;
	}
	return 0;
}

void FFmpegCond::Signal()
{
	event->Trigger();
}

int FFmpegCond::Wait(FCriticalSection* mutex)
{
	mutex->Unlock();
	event->Wait();
	mutex->Lock();
	return 0;
}

int FFmpegCond::WaitTimeout(FCriticalSection* mutex, int32 timeoutMilliseconds)
{
	mutex->Unlock();
	bool bResult = event->Wait(timeoutMilliseconds);
	mutex->Lock();
	return bResult ? 0 : -1;
}
