#include "WKMessageQueue.h"
#include "WKMessage.h"
#include "WKLooper.h"
#include "WKHandler.h"

#include <unistd.h>

#include <utils.h>

#define MODULE_NAME "WKMESAAGE_TEST"

using namespace android;

int main(void)
{
	int cnt = 0;
	
	WKHandler *handler = new WKHandler();
	WKLooper  *looper  = new WKLooper();
	handler->setLooper(looper);
	looper->loop();

	while(1)
	{
		WKMessage *msg = WKMessage::obain(cnt++, handler);
		handler->sendMessage(msg);
		if(cnt >10)
			break;
		usleep(1000000);
	}

	//等待处理完
	sleep(10);
	LOGD("now,sPoolSize:%d,sPool is %s.",WKMessage::sPoolSize,WKMessage::sPool==NULL?"null":"not null");
}



















