#include "WKMessage.h"
#include "WKMessageQueue.h"
#include <Looper.h>
#include <threads.h>


#include "utils.h"


#define MODULE_NAME "SKMESAAGE_QUEUE"

using namespace android;

WKMessageQueue::WKMessageQueue()
{
	mLooper = new Looper();
	mQuiting = false;
	mMessages = NULL;
}
PT_WKMessage WKMessageQueue::next()
{
	int nextPollTimeoutMillis = 0;

	for (;;)
	{
        if(NULL == mLooper)
        {
            return NULL;
        }
        
		mLooper->pollOnce(nextPollTimeoutMillis);
		{
			// Try to retrieve the next message.  Return if found.
			long now = uptimeMillis();
			PT_WKMessage msg = mMessages;
			if (NULL != msg)
			{
				long when = msg->when;
				if (now >= when)
				{
					//:暂不使用空闲线程处理任务，当无消息时消息循环必阻塞
					// mBlocked = false;
					mMessages = msg->next;
					msg->next = NULL;
					msg->markInUse();
					return msg;
				}
				else
				{
//		    nextPollTimeoutMillis = (int) Math.min(when - now, Integer.MAX_VALUE);
					nextPollTimeoutMillis = (int) (when - now);
				}
			}
			else
			{
				nextPollTimeoutMillis = -1;
			}
		}
	}
}

bool WKMessageQueue::enqueueMessage(PT_WKMessage msg, long when)
{
	bool needWake = false;
	{
		if (mQuiting)
		{
			LOGI("sending message to a handler on a dead thread");
			return false;
		}
		else if (msg->handler == NULL)//:msg.handler == NULL特殊专用消息，用以通知退出
		{
			mQuiting = true;
		}
		msg->when = when;
		PT_WKMessage p = mMessages;
		if (p == NULL || when < p->when)
		{
			msg->next = p;
			mMessages = msg;
			//needWake = mBlocked; // new head, might need to wake up
			needWake = true;
		}
		else
		{
			PT_WKMessage prev = NULL;
			while (p != NULL && p->when <= when)
			{
				prev = p;
				p = p->next;
			}
			msg->next = prev->next;
			prev->next = msg;
			needWake = false; // still waiting on head, no need to wake up
		}
	}
	if (needWake)
	{
		mLooper->wake();
	}
	return true;
}


bool WKMessageQueue::removeAllMessages()
{
    AutoMutex _l(mLock);
    PT_WKMessage p = mMessages;

    while(p != NULL)
    {
        PT_WKMessage n = p->next;
        mMessages = n;
        p->recycle();
		p = n;
    }

    return true;

}

bool WKMessageQueue::removeMessages(int what)
{
	PT_WKMessage p = mMessages;
	bool found = false;

	// Remove all messages at front.
	while (p != NULL && p->what == what)
	{
		found = true;
		PT_WKMessage n = p->next;
		mMessages = n;
		p->recycle();
		p = n;
	}

	// Remove all messages after front.
	while (p != NULL)
	{
		PT_WKMessage n = p->next;
		if (n != NULL)
		{
			if (n->what == what)
			{
				found = true;
				PT_WKMessage nn = n->next;
				n->recycle();
				p->next = nn;
				continue;
			}
		}
		p = n;
	}

	return found;
}

bool WKMessageQueue::findMessage(int what)
{
	PT_WKMessage p = mMessages;
	while (p != NULL)
	{
		if (p->what == what)
			return true;
		p = p->next;
	}
	return false;
}

WKMessageQueue::~WKMessageQueue()
{
	//:do nothing
	mLooper = NULL;
	mMessages = NULL;		// 只要头销毁了，链表后续的元素由于没有指针指向它了，也会随之销毁
}

