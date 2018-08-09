#include "WKTSPackage.h"
#include "WKHandler.h"
#include "wk_android_std.h"
#include <string.h>


#define MODULE_NAME "WKTSPACKAGE"
using namespace android;
Mutex WKTSPackage::mLock;
int WKTSPackage::sPoolSize = 0;
WKTSPackage* WKTSPackage::sPool = NULL;

WKTSPackage* WKTSPackage::obain()
{
	AutoMutex _l(mLock);
	if (sPool != NULL)
	{
		WKTSPackage* m = sPool;
		sPool = m->next;
		m->next = NULL;
		sPoolSize--;
		return m;
	}
	return new WKTSPackage();
}
 WKTSPackage* WKTSPackage::obain(WKHandler *handler)
{
	WKTSPackage* m = obain();
	m->handler = handler;
	return m;
}
void WKTSPackage::clearForRecycle()
{
	memset(data, 0, sizeof(data));
	handler = NULL;
	next = NULL;
	mIsInUse = false;
}
void WKTSPackage::recycle()
{
    AutoMutex _l(mLock);	    //:mutex
	clearForRecycle();
	if (sPoolSize < MAX_POOL_SIZE)
	{
		next = sPool;
		sPool = this;
		sPoolSize++;
	}
}
bool WKTSPackage::isInUse()
{
	return mIsInUse;
}
void WKTSPackage::markInUse()
{
	mIsInUse = true;
}
//:code for test
int WKTSPackage::gtestid = 0;
WKTSPackage::WKTSPackage()
{
	handler = NULL;
	next = NULL;
	memset(data, 0, sizeof(data));
	mIsInUse = false;
	mHoldSelf = this;
	// code for debug
	// testid = ++gtestid;
	// LOGD("construct ts package %d",testid);
}
WKTSPackage::~WKTSPackage()
{
	handler = NULL;
	next = NULL;
	// LOGD("destroy ts package %d",testid);
}
unsigned short WKTSPackage::getPID()
{
	return (((data[1] & 0x1F) << 8) | data[2]);
}
//:end
