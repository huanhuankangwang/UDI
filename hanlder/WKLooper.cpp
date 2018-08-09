#include "WKMessageQueue.h"
#include "WKMessage.h"
#include "WKLooper.h"
#include "WKHandler.h"

#include <utils.h>


#define MODULE_NAME "WKLOOPER"

using namespace android;

WKLooper::WKLooper()
{
	mQueue = NULL;
	mThreadId = (pthread_t)NULL;
}

WKLooper::~WKLooper()
{
	if(NULL != mQueue)
	{
        delete mQueue;
    }
    
    mQueue = NULL;
}

bool WKLooper::loop()
{
	mQueue = new WKMessageQueue();
	// +1ǿָ�룬��ֹthis�ڴ��ݹ����У���δ������ʱ������

	int ret = pthread_create(&mThreadId, NULL, &looper_thread, this);
	if (ret == 0)
	{
		// mQueue = new SKMessageQueue();
		LOGI("looper create success!");
		return true;
	}
	else
	{
		LOGE("looper create thread fails");
		return false;
	}
}

WKMessageQueue* WKLooper::getMessageQueue()
{
	return mQueue;
}

void* WKLooper::looper_thread(void* arg)
{
	WKLooper*  plooper = static_cast<WKLooper*>(arg);
	WKLooper* looper = plooper;
	// �Ѿ�����looper����decStrong��
	WKMessageQueue* queue = looper->getMessageQueue();

	while (true)
	{
		if(NULL == queue)
		{
			LOGI("message queue null, thread %ld exit...", pthread_self());
			return NULL;
		}

		WKMessage* msg = queue->next(); // might block
		if (NULL != msg)
		{

			if (msg->handler == NULL)
			{
				// No target is a magic identifier for the quit message.
				LOGI("thread %ld exit...", pthread_self());
				return NULL;
			}
			msg->handler->dispatchMessage(msg);
			msg->recycle();//:��ʱ��Ϣ�Ѿ�����Ϣ������ȡ����ֻ��Ҳ������������գ����赣��������recycle��remove���������ظ�����
		}
	}
	return NULL;
}

void WKLooper::doEventLoop()
{
	mQueue = new WKMessageQueue();
	looper_thread(this);
}

