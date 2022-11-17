// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <HAL/RunnableThread.h>
#include <functional>
#include <HAL/Runnable.h>
/**
 * 基于UE4异步API实现
 * 用于替换SDL_CreateThread
 */
class LambdaFunctionRunnable : public FRunnable
{
public:
	/**
	* 运行一个异步线程
	* threadName 线程名称
	* f 要执行的函数
	*/
	static FRunnableThread* RunThreaded(FString threadName, std::function<void()> f);
	/** 
	* 退出
	*/
	void Exit() override;
	/** 
	* 运行
	*/
	uint32	Run()	override;
protected:
	/**
	* 构造函数
	*/
	LambdaFunctionRunnable(std::function<void()> f);
	std::function<void()> _f;
	FRunnableThread* thread;
};
