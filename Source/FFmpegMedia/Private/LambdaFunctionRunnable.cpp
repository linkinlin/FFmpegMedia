// Fill out your copyright notice in the Description page of Project Settings.


#include "LambdaFunctionRunnable.h"

/**
* @brief 运行一个线程
* @param threadName 线程名称
* @param threadFunc 线程执行方法
* @return
*/
FRunnableThread* LambdaFunctionRunnable::RunThreaded(FString threadName, std::function<void()> threadFunc)
{
	static int currentThread = 0;
	LambdaFunctionRunnable* runnable = new LambdaFunctionRunnable(threadFunc); //创建自定义线程对象
	FString _threadName = threadName + FString::FromInt(currentThread++); //构建线程名称
	runnable->thread = FRunnableThread::Create(runnable, *_threadName); //创建线程，并返回引用
	return  runnable->thread;
}

/**
 * @brief 构造器
 * @param threadFunc 线程执行方法
*/
LambdaFunctionRunnable::LambdaFunctionRunnable(std::function<void()> threadFunc) {
	_threadFunc = threadFunc;
}

/**
* @brief 执行线程
* @return
*/
uint32 LambdaFunctionRunnable::Run() {
	_threadFunc();
	return 0;
}

/**
* @brief 退出线程
*/
void LambdaFunctionRunnable::Exit() {
	delete this;
}
