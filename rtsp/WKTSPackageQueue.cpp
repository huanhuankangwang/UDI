
#include "WKTSPackage.h"
#include "WKTSPackageQueue.h"
#include "wk_android_std.h"
#define MODULE_NAME "SKTSPACKAGE_QUEUE"

using namespace android;

WKTSPackageQueue::WKTSPackageQueue()
{
	mHead = NULL;
	mEnd = NULL;
    mSize = 0;
}
WKTSPackage* WKTSPackageQueue::preNext()
{
	AutoMutex _l(mLock);
	return mHead;
}
WKTSPackage* WKTSPackageQueue::next()
{
	AutoMutex _l(mLock);
	WKTSPackage* pkg = mHead;
	if (pkg != NULL)
	{
		if (pkg == mEnd)
		{
			mHead = mEnd = NULL;	// 取走最后一个包
		}
        else
        {
		    mHead = pkg->next;
        }
		pkg->next = NULL;
		pkg->markInUse();
        mSize--;
	}
    //LOGD("next, mSize:%d", mSize);
	return pkg;
}
int WKTSPackageQueue::getTsPkgSize()
{
	AutoMutex _l(mLock);
#if 0
	WKTSPackage* p = mHead;
	int i = 0;
	while (p != NULL)
	{
		i++;
		p = p->next;
	}

    LOGD("TsPkgSize, i = %d, mSize = %d", i, mSize);
#endif
	return mSize;
}

void WKTSPackageQueue::enqueuePackage(WKTSPackage* pkg)
{
	AutoMutex _l(mLock);
    pkg->next = NULL;
    
	if (mEnd == NULL)		// 第一个数据
	{
		mHead = mEnd = pkg;
	}
    else
	{
		mEnd->next = pkg;
		mEnd = pkg;
	}

    mSize++;
    //LOGD("enqueuePackage, mSize:%d", mSize);
}
void WKTSPackageQueue::removeAll()
{
	WKTSPackage* p = next();
	while (p != NULL)
	{
		p->recycle();
        if(p == mEnd)
        {
            break;
        }
		p = next();
	}

    AutoMutex _l(mLock);
    mSize = 0;
}
WKTSPackageQueue::~WKTSPackageQueue()
{
	//:do nothing
	mHead = NULL;
	mEnd = NULL;
    mSize = 0;
}




















