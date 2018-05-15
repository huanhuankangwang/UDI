#ifndef H_QZ_HANDLER
#define H_QZ_HANDLER

//#include <WKLooper.h>
//#include <WKMessage.h>
 

//:每个handler绑定一个looper，每个looper绑定一个message queue
//:向指定的线程发送消息必须获取这个线程的handler，然后通过handler发送
namespace android
{
class WKLooper;
class WKMessage;

class WKHandler
{
public:
	WKHandler();
	void setLooper(WKLooper* looper);
	void sendMessage(WKMessage* msg);
	void sendMessage(WKMessage* msg, long when);
	WKMessage* obainMessage(int what);
	WKMessage* obainMessage(int what, int arg1, int arg2);
	WKMessage* obainMessage(int waht, int arg1, int arg2, void* obj);
	WKMessage* obainMessage(int what, int arg1);
	WKMessage* obainMessage();
	virtual void dispatchMessage(WKMessage* msg);
	virtual ~WKHandler();
private:
	WKLooper* mLooper;
};
}
#endif //:H_QZ_HANDLER









