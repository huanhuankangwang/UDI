#ifndef H_SK_TSPACKAGE_QUEUE
#define H_SK_TSPACKAGE_QUEUE


#include <threads.h>
namespace android
{
class WKTSPackage;
class WKTSPackageQueue
{
public:
	WKTSPackageQueue();
	virtual ~WKTSPackageQueue();
	/* 如果TS包中无数据了，将会返回NULL */
	WKTSPackage* next();
	WKTSPackage* preNext(); /* 试探性取出头部，不删除头部的TS包 */
	void enqueuePackage(WKTSPackage* pkg);
	//:销毁消息队列中所有同类型的消息，防止操作过快导致频繁处理
	void removeAll();
	int getTsPkgSize();
private:
	WKTSPackage* mHead;
	WKTSPackage* mEnd;
	mutable Mutex mLock;
	int mSize;
};
}

#endif //H_SK_TSPACKAGE_QUEUE












