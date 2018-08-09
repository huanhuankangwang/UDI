#include "WKMessage.h"
#include "WKLooper.h"
#include "WKHandler.h"
#include "WKMessageQueue.h"

#include "utils.h"

#define MODULE_NAME "WKHANDLER"
using namespace android;

WKHandler::WKHandler()
{
	//mLooper = looper;
	mLooper = NULL;
}
void WKHandler::setLooper(WKLooper* looper)
{
	mLooper = looper;
}
void WKHandler::sendMessage(WKMessage* msg)
{
	sendMessage(msg, uptimeMillis());
}
void WKHandler::sendMessage(WKMessage *msg, long when)
{
	if (mLooper == NULL)
	{
		//:TODO
		//:如果looper为空，获取当前线程的looper作为默认looper
		LOGE("there is no looper to send,you should set looper first.");
		return;
	}
	WKMessageQueue* queue = mLooper->getMessageQueue();
	if (queue == NULL)
	{
		LOGE("there is no message queue and looper deal with msg.");
		return;
	}
	//queue->enqueueMessage(msg,uptimeMillis());
	queue->enqueueMessage(msg, when);
}
void WKHandler::dispatchMessage(PT_WKMessage msg)
{
	LOGI("msg->what =%d msg->when=%ld",msg->what,msg->when);
	return;
}
WKMessage* WKHandler::obainMessage(int what)
{
	return WKMessage::obain(0, what, 0, 0, NULL, this);
}
WKMessage* WKHandler::obainMessage(int what, int arg1, int arg2)
{
	return WKMessage::obain(0, what, arg1, arg2, NULL, this);
}
WKMessage* WKHandler::obainMessage(int what, int arg1, int arg2, void* obj)
{
	return WKMessage::obain(0, what, arg1, arg2, obj, this);
}
WKMessage* WKHandler::obainMessage(int what, int arg1)
{
	return WKMessage::obain(0, what, arg1, 0, NULL, this);
}
WKMessage* WKHandler::obainMessage()
{
	return WKMessage::obain();
}
WKHandler::~WKHandler()
{
	mLooper = NULL;
}
