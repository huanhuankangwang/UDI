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
	//:销毁消息队列中所有同类型的消息，防止操作过快导致频繁处理
	bool removeMessages(int what);
	bool removeAllMessages();
	bool findMessage(int what);
private:
	WKMessage *mMessages;
	//Looper mLooper;//:每个消息队列必须有一个looper来控制，looper随着消息队列的创建而创建，销毁而销毁，所以此处直接使用成员变量，不用指针
	Looper *mLooper;
	bool mQuiting;
	//:暂不使用空闲线程处理任务，当无消息时消息循环必阻塞
	//boolean mBlocked;
	Mutex mLock;
};
}

#endif //H_QZ_MESSAGE_QUEUE
