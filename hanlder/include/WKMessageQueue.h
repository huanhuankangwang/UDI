#ifndef H_QZ_MESSAGE_QUEUE
#define H_QZ_MESSAGE_QUEUE

//#include <WKMessage.h>

#include <threads.h>
#include <pthread.h>

namespace android
{
class WKMessage;
class Looper;
class Mutex;

class WKMessageQueue
{
public:
	WKMessageQueue();
	virtual ~WKMessageQueue();
	WKMessage* next();
	bool enqueueMessage(WKMessage *msg, long when);
	//:������Ϣ����������ͬ���͵���Ϣ����ֹ�������쵼��Ƶ������
	bool removeMessages(int what);
	bool removeAllMessages();
	bool findMessage(int what);
private:
	WKMessage *mMessages;
	//Looper mLooper;//:ÿ����Ϣ���б�����һ��looper�����ƣ�looper������Ϣ���еĴ��������������ٶ����٣����Դ˴�ֱ��ʹ�ó�Ա����������ָ��
	Looper *mLooper;
	bool mQuiting;
	//:�ݲ�ʹ�ÿ����̴߳������񣬵�����Ϣʱ��Ϣѭ��������
	//boolean mBlocked;
	Mutex mLock;
};
}

#endif //H_QZ_MESSAGE_QUEUE
