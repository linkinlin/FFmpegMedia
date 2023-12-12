// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <HAL/RunnableThread.h>
#include <functional>
#include <HAL/Runnable.h>

/**
 * 自定义线程类
 * 用于替换SDL_CreateThread
 */
class LambdaFunctionRunnable : public FRunnable
{
public:

	/**
	 * @brief 运行一个线程
	 * @param threadName 线程名称
	 * @param threadFunc 线程执行方法 
	 * @return 
	*/
	static FRunnableThread* RunThreaded(FString threadName, std::function<void()> threadFunc);
	
	/**
	 * @brief 退出线程
	*/
	void Exit() override;
	
	/**
	 * @brief 执行线程
	 * @return 
	*/
	uint32	Run()	override;

protected:
	/**
	 * @brief 构造函数
	 * @param threadFunc 线程执行方法
	*/
	LambdaFunctionRunnable(std::function<void()> threadFunc);

	//线程执行方法
	std::function<void()> _threadFunc;

	//线程
	FRunnableThread* thread;
};
