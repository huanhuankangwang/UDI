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
	 *��Ϣһ�������ս������Ϣ��Я�������ݣ����պ���粻Ӧ��ʹ����ϢЯ�����κ�����
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
	//:��Ϣ�����Ա����
	long when;
	int what;
	int arg1;
	int arg2;
	//:��Ϣϵͳ����������obj��Ա��ֻ�����ʱ�����ÿգ�ʹ��ʱע������
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
















