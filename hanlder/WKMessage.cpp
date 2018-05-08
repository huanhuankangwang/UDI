//#include <WKHandler.h>

#include <WKMessage.h>
#include <threads.h>

#include <utils.h>


using namespace android;

#define MODULE_NAME "WKMESAAGE"


Mutex WKMessage::mLock;
int WKMessage::sPoolSize = 0;
PT_WKMessage WKMessage::sPool = NULL;

PT_WKMessage WKMessage::obain()
{
	AutoMutex _l(mLock);
	if (sPool != NULL)
	{
		PT_WKMessage m = sPool;
		sPool = m->next;
		m->next = NULL;
		sPoolSize--;
		return m;
	}
    
	return new WKMessage();
}

PT_WKMessage WKMessage::obain(int what,WKHandler* handler)
{
	return obain(uptimeMillis(), what, 0, 0, NULL, handler);
}

PT_WKMessage WKMessage::obain(long when, int what, int arg1, int arg2, void* obj, WKHandler* handler)
{
	PT_WKMessage m = obain();
	m->when = when;
	m->what = what;
	m->arg1 = arg1;
	m->arg2 = arg2;
	m->obj = obj;
	m->handler = handler;
	return m;
}
PT_WKMessage WKMessage::obain(WKHandler* handler)
{
	PT_WKMessage m = obain();
	m->handler = handler;
	return m;
}
void WKMessage::clearForRecycle()
{
	when = 0;
	what = 0;
	arg1 = 0;
	arg2 = 0;
	obj = NULL;
	handler = NULL;
	next = NULL;
	mIsInUse = false;
}
void WKMessage::recycle()
{
	clearForRecycle();
	AutoMutex _l(mLock);	    //:mutex
	if (sPoolSize < MAX_POOL_SIZE)
	{
		next = sPool;
		sPool = this;
		sPoolSize++;
	}
}
bool WKMessage::isInUse()
{
	return mIsInUse;
}
void WKMessage::markInUse()
{
	mIsInUse = true;
}
//:code for test
int WKMessage::gtestid = 0;
WKMessage::WKMessage()
{
	when = 0;
	what = 0;
	arg1 = 0;
	arg2 = 0;
	obj = NULL;
	handler = NULL;
	next = NULL;
	mIsInUse = false;

	// code for debug
	// testid = ++gtestid;
	LOGD("construct msg %d",testid);
}
WKMessage::~WKMessage()
{
	handler = NULL;
	next = NULL;
	LOGD("destroy msg %d",testid);
}
//:end
