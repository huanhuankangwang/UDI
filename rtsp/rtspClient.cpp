#include <rtspClient.h>

#include <WKLooper.h>
#include <WKHandler.h>
#include <WKSessionContext.h>
#include <WKStreamCtrl.h>
#include <WKMessage.h>
#include <stdio.h>

using namespace android;
/*

export LD_LIBRARY_PATH=/home/wangkang/workdir/UDI/lib/:$LD_LIBRARY_PATH

*/
WKRtspClient::WKRtspClient()
{
	mContex = new WKSessionContext();
	mStreamCtrl = new WKStreamCtrl(mContex);
	mHandler = new WKRtspClientHandler();

	mHandler->setLooper(this);
}

WKRtspClient::~WKRtspClient()
{
	delete mContex;
	delete mStreamCtrl;
	mContex = NULL;
	mStreamCtrl = NULL;
}
void WKRtspClient::doEventLoop()
{
	WKLooper::doEventLoop();
}
WKRtspClientHandler::WKRtspClientHandler()
{
}

WKRtspClientHandler::~WKRtspClientHandler()
{
}
void WKRtspClientHandler::dispatchMessage(WKMessage* msg)
{
	switch(msg->what)
	{
		default:
			break;
	}
}

int main(int argc,char **argv)
{
	WKRtspClient *rtspClient = new WKRtspClient();

	
	if(rtspClient)
		rtspClient->doEventLoop();
	printf("usage:parser\r\n");
	return 0;
}
