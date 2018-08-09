#ifndef H_QZ_LOOPER
#define H_QZ_LOOPER

//#include <WKMessageQueue.h>
#include <pthread.h>

namespace android
{
class WKMessageQueue;

class WKLooper
{
public:
	WKLooper();
	//:�´�����looper���������ִ��loop������ſ�������ʹ��
	bool loop();
	WKMessageQueue* getMessageQueue();
	virtual ~WKLooper();

	static void* looper_thread(void* arg);
	void doEventLoop();
private:
	pthread_t mThreadId;
	WKMessageQueue* mQueue;
};
}

#endif //:H_QZ_LOOPER
