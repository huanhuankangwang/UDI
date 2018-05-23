#ifndef  __TIMER_TASK_H__
#define  __TIMER_TASK_H__
#include  <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h> 
#include <sys/time.h>
#include <string.h> 
#include <pthread.h> 

typedef void (*pThreadFunc)(void *arg );
typedef struct TimerOps{
	int (*start)(struct TimerOps*);
	int (*changeParameter)(struct TimerOps*,int,int,int);
	int (*changeArg)(struct TimerOps*,void *,int);
	int (*reset)(struct TimerOps*);
	int (*stop)(struct TimerOps*);
}TimerOps,*pTimerOps;
/*  fun:创建定时器任务
 *  startTime:启动时间单位ms
 *  loopTime:循环时间 单位ms
 *  loopCount:循环次数
 *  startThread: 0不启动线程处理 1启动线程处理
 *  taskFunc:线程任务函数
 *  arg:线程任务函数中的形参
 *  argLen:线程任务函数新参的长度
 *
 *
 * */
pTimerOps createTimerTaskServer( int startTime, int loopTime,int loopCount,int startThread,pThreadFunc taskFunc,void *arg,int dataLen);
void destroyTimerTaskServer(pTimerOps *timerServer );


#endif
