#include "WKStream.h"

#include "WKURLParser.h"
#include "WKRtspCtrl.h"
#include "WKUDP.h"
#include "WKSessionContext.h"
#include "wk_android_std.h"
#include "WKMessage.h"
#include "WKLooper.h"
#include <time.h>

#include "WKMediaProcessor.h"

#include <String.h>
#include <stdlib.h>
#include <string.h>


using namespace android;

WKStream::WKStream(WKSessionContext* context)
{
	mContext = context;
	mUrlParser = new WKURLParser();
}

WKStream::~WKStream()
{
}
void WKStream::setTsHandler(WKHandler* h)
{
	mContext->setTsHandler(h);
}

void WKStream::setEventHandler(WKHandler* h)
{
	mContext->setEventHandler(h);
}

bool WKStream::isChannel()
{
	// if(mRtspCtrl->getCurrentStreamType() == SK_STREAM_MODE_VOD)
	if ((mContext->getStreamType() & WK_STREAM_CHANNEL) == 0)
		return false;
    
	return true;
}
bool WKStream::isLive()
{	
    if ((mContext->getStreamType() & WK_STREAM_LIVE) == 0)
		return false;
    
	return true;
}
void WKStream::ctlBufferState(int burstMode)
{
	// mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_GETPARAMETER,burstMode,0));
	// mContext->mBurstMode = burstMode;
	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
	{

    }
    else
    {
    }
	// LOGD("stream ctrl send burst");
}


int WKStream::getTimeshiftLength()
{
    if(mUrlParser == NULL)
        return 0;

    return mUrlParser->getTimeShiftLength();
}

bool WKStream::isMultiLive()
{   
    if(mUrlParser == NULL)
        return false;

    return mUrlParser->isMultiLive();
}

bool WKStream::trickAble()
{
    bool ret = false;
    
    if(mUrlParser != NULL)
        ret = mUrlParser->mTrickable;

    return ret;
}
int WKStream::getCurrentProtocolType()
{
    if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
    {
        return STM_PROTO_MULTICAST;
    }
    else
    {
        if((mContext->getStreamType() & WK_STREAM_UDP) > 0)
        {
            if(mContext->getHasRtp())
            {
                return STM_PROTO_UNICAST_UDP_RTP;
            }
            else
            {
                return STM_PROTO_UNICAST_UDP;
            }
        }
        else
        {
            if(mContext->getHasRtp())
            {
                return STM_PROTO_UNICAST_TCP_RTP;
            }
            else
            {
                return STM_PROTO_UNICAST_TCP;
            }
        }
    }

    return STM_PROTO_UNKNOWN;
}

void WKStream::dispatchMessage(PT_WKMessage msg)
{
	LOGI("msg->what =%d msg->when=%ld",msg->what,msg->when);
	return;
}

