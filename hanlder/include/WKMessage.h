#ifndef H_QZ_MESSAGE
#define H_QZ_MESSAGE

//#include <WKHandler.h>
#include <pthread.h>

/**
 *
 **/

namespace android
{
class WKHandler;
class Mutex;

typedef class WKMessage
{
public:
	static WKMessage* obain();
	static WKMessage* obain(long when, int what, int arg1, int arg2, void* obj, WKHandler* handler);
	static WKMessage* obain(int what,WKHandler* handler);
	static WKMessage* obain(WKHandler* handler);
	/*
	 *消息一旦被回收将清除消息所携带的数据，回收后外界不应再使用消息携带的任何数据
	 */
	void recycle();
	bool isInUse();
	void markInUse();
	//:code for test
	WKMessage();
	virtual ~WKMessage();
	int testid;
	static int gtestid;
	//:end
private:
	void clearForRecycle();
public:
	//:消息对象成员变量
	long when;
	int what;
	int arg1;
	int arg2;
	//:消息系统不负责清理obj成员，只会回收时把他置空，使用时注意清理
	void *obj;
	WKHandler* handler;

	static WKMessage* sPool;
	static int sPoolSize;
private:
	friend class WKMessageQueue;

	const static int MAX_POOL_SIZE = 1000;

	WKMessage* next;
	//mutable Mutex mLock;
	static Mutex mLock;
	bool mIsInUse;
}*PT_WKMessage;

};



#endif //:H_QZ_MESSAGE
















