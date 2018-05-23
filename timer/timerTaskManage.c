#include "timerTaskManage.h"
#include "common/debugLog.h"

typedef struct  TimerServer{
		TimerOps ops;
		timer_t timerid;
		pThreadFunc handle;
		int startTime;
		int loopTime;
		int loopCount; //循环总次数
		int remainCount; //剩下的循环次数
		void *handleArg;
		int handleArgLen;
		int startThread;
		pthread_mutex_t  mutex;
}TimerServer,*pTimerServer;

static int start(struct TimerOps* base);
static int changeParameter (struct TimerOps* base,int startTime, int loopTime,int loopCount);
static int changeArg(struct TimerOps* base,void *data,int size);

static int stop(struct TimerOps* base);
static int reset(struct TimerOps* base);
static void notify_function(union sigval signo);



TimerOps timerOps = {
	.start = start,
	.changeParameter = changeParameter,
	.changeArg = changeArg,
	.reset = reset,
	.stop = stop,
};
static int start(struct TimerOps* base)
{
	pTimerServer timerServer = (pTimerServer)base;
	struct sigevent hard_evp;;
	if(timerServer == NULL)
	{
		return -1;
	}
	struct itimerspec it;
	pthread_mutex_lock(&timerServer->mutex);

	if(timerServer->timerid  == NULL)
	{
		memset(&hard_evp, 0, sizeof(struct sigevent));
	    hard_evp.sigev_value.sival_ptr = timerServer;
	    hard_evp.sigev_notify = SIGEV_THREAD;
	    hard_evp.sigev_notify_function = notify_function;
	    hard_evp.sigev_notify_attributes = NULL;//&attr;
	    if (timer_create(CLOCK_MONOTONIC, &hard_evp, &(timerServer->timerid)) == -1)
	    {
	      
	        pthread_mutex_unlock(&timerServer->mutex);
		
	        goto fail0;
	    }
	}

	if(timerServer->loopTime <= 0 ){
		it.it_interval.tv_sec =  0;
		it.it_interval.tv_nsec = 1000*1000*500LL; 
	}else{
		it.it_interval.tv_sec =  (timerServer->loopTime/1000);
	    it.it_interval.tv_nsec = (timerServer->loopTime%1000)*1000*1000LL;
	}
	it.it_value.tv_sec =  (timerServer->startTime/1000);
	if(timerServer->startTime <= 0) {
		it.it_value.tv_nsec = 1;
	}else
	    it.it_value.tv_nsec = (timerServer->startTime%1000)*1000*1000LL;

	timerServer->remainCount = timerServer->loopCount;
	if (timer_settime(timerServer->timerid, 0, &it, NULL) == -1)
	{
	       
	        pthread_mutex_unlock(&timerServer->mutex);
			
	        goto fail0;
	}
	pthread_mutex_unlock(&timerServer->mutex);

	
	return 0;
fail0:
	return -1;
}
static int changeArg(struct TimerOps* base,void *data,int size)
{
	pTimerServer timerServer = (pTimerServer)base;
	if(timerServer == NULL)
	{
			return -1;
	}
	pthread_mutex_lock(&timerServer->mutex);
	
	if(timerServer->handleArg != NULL&&timerServer->handleArgLen >0)
	{
		free(timerServer->handleArg);
		timerServer->handleArg = NULL;
		timerServer->handleArgLen = 0;
	}else {
		if( size <= 0)
		{	
			timerServer->handleArgLen = 0;
			timerServer->handleArg = data;
			
		}else {
			timerServer->handleArg  = malloc(size);
			if(timerServer->handleArg == NULL)
			{
				
				pthread_mutex_unlock(&timerServer->mutex);
			
				return -1;
			}
			bzero(timerServer->handleArg,size);
			memcpy(timerServer->handleArg,data,size);
	
		}

	}
	pthread_mutex_unlock(&timerServer->mutex);
	
	return 0;
}
static int changeParameter (struct TimerOps* base,int startTime, int loopTime,int loopCount)
{
	pTimerServer timerServer = (pTimerServer)base;
	if(timerServer == NULL)
	{
			return -1;
	}
	
	pthread_mutex_lock(&timerServer->mutex);

	timerServer->startTime = startTime;
	timerServer->loopCount = loopCount ;
	timerServer->loopTime = loopTime;
	pthread_mutex_unlock(&timerServer->mutex);




	return 0;
}
static int stop(struct TimerOps* base)
{
	pTimerServer timerServer = (pTimerServer)base;
	if(timerServer == NULL)
	{
		return -1;
	}

	pthread_mutex_lock(&timerServer->mutex);

	if( timerServer->timerid !=  NULL){

		timer_delete(timerServer->timerid);
	}

	timerServer->timerid = NULL;
	pthread_mutex_unlock(&timerServer->mutex);

	return 0;
	fail0:
		return -1;
}
static int reset(struct TimerOps* base)
{

	return stop(base)|| start(base);
}
pTimerOps createTimerTaskServer( int startTime, int loopTime,int loopCount,int startThread,pThreadFunc taskFunc,void *arg,int dataLen)
{

	struct sigevent hard_evp;
	pTimerServer timerServer =  (pTimerServer) malloc(sizeof(	struct  TimerServer));
	if(timerServer == NULL)
		goto fail0;
	bzero(timerServer,sizeof(struct  TimerServer));
	timerServer->handle = taskFunc;

	timerServer->handleArgLen = (dataLen >= 0?dataLen:0);
	if(dataLen > 0 ){
		timerServer->handleArg = malloc(dataLen);
		if(timerServer->handleArg == NULL)
			goto fail1;
		memcpy(timerServer->handleArg,arg,dataLen);
	}else {
		timerServer->handleArg = arg;
	}

	timerServer->loopCount = loopCount;
	memset(&hard_evp, 0, sizeof(struct sigevent));
    hard_evp.sigev_value.sival_ptr = timerServer;
    hard_evp.sigev_notify = SIGEV_THREAD;
    hard_evp.sigev_notify_function = notify_function;
    hard_evp.sigev_notify_attributes = NULL;//&attr;
    if (timer_create(CLOCK_MONOTONIC, &hard_evp, &(timerServer->timerid)) == -1)
    {  
        LOGE("fail to timer_create");
        goto fail2;
    }
    pthread_mutex_init(&timerServer->mutex,NULL);


    timerServer->loopCount = loopCount;
    timerServer->remainCount = loopCount;
    timerServer->loopTime = loopTime;
    timerServer->startTime = startTime;
    timerServer->ops = timerOps;
	timerServer->startThread = startThread;
 	return (pTimerOps)timerServer;
fail3:
 		timer_delete(timerServer->timerid);
fail2:
		free(timerServer->handleArg);
		timerServer->handleArg = NULL;
fail1:
 		free(timerServer);
fail0:
	return NULL;

}

static void notify_function(union sigval signo)
{
	pTimerServer	timerServer = signo.sival_ptr;
	//每次自动释放
	if(timerServer == NULL)
		return;

	if( (timerServer->remainCount == 0) || (timerServer->remainCount < -1))
	{
		
		pthread_mutex_lock(&timerServer->mutex);
	
		if(timerServer->timerid != NULL )
			timer_delete(timerServer->timerid);
		timerServer->timerid = NULL;
		pthread_mutex_unlock(&timerServer->mutex);
	
		return;	
	}
	if( timerServer->remainCount > 0)
		timerServer->remainCount --;
	
	if( timerServer->handle != NULL){
		pthread_t pid;

	if(timerServer->startThread  != 0){
		pthread_create(&pid,NULL,timerServer->handle,timerServer->handleArg);
		pthread_detach(pid);
	}else {
		timerServer->handle(timerServer->handleArg);
	}
	}
	

}

void destroyTimerTaskServer(pTimerOps *timerServer )
{	
	pTimerServer  tempP = (pTimerServer)(*timerServer);
	if((tempP!=NULL)){
		if(tempP->timerid != NULL ){
			pthread_mutex_lock(&tempP->mutex);
			timer_delete(tempP->timerid);
			tempP->timerid = NULL;
			pthread_mutex_unlock(&tempP->mutex);
		}
		if(tempP->handleArg != NULL &&tempP->handleArgLen > 0)
				free(tempP->handleArg);
		tempP->handleArg = NULL;
	
		pthread_mutex_lock(&tempP->mutex);
		tempP->loopCount = 0;
		pthread_mutex_unlock(&tempP->mutex);

		pthread_mutex_destroy(&tempP->mutex);
		free(tempP);
		*timerServer = NULL;
	}
}












