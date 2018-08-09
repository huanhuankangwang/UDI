#include "WKStreamCtrl.h"
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

#define MODULE_NAME "WKIPTV_STREAM_CTRL"
using namespace android;

static String strErrorUrl;

WKStreamCtrl::WKStreamCtrl(WKSessionContext* context):WKStream(context)
{
	//mUrlParser = new WKURLParser();
	mRtspCtrl = new WKRtspCtrl();
	mUDPCtrl = new WKUDP();
	mContext = context;
	mContext->setRtspHandler(mRtspCtrl);
	mRtspCtrl->setContext(mContext);
    mRtspCtrl->setStream(this);
	mRtspCtrl->setUDPCtrl(mUDPCtrl);
    mUDPCtrl->setRtspCtrl(mRtspCtrl);
	mLooper = new WKLooper();
	if (mLooper->loop())
	{
		setLooper(mLooper);
		LOGI("WKStreamCtrl create looper success!");
	}
	else
		LOGE("WKStreamCtrl create looper falid!");
}

WKStreamCtrl::~WKStreamCtrl()
{
	LOGE("WKStreamCtrl destroy!"); 
	mUrlParser = NULL;

	mUDPCtrl = NULL;
	mRtspCtrl = NULL;

	mContext = NULL;
	if (mLooper != NULL)
	{
		sendMessage(WKMessage::obain(NULL));
	}
	mLooper = NULL;

}


void WKStreamCtrl::dispatchMessage(WKMessage* msg)
{
	//
}

/*
void WKStreamCtrl::setTsHandler(WKHandler* h)
{
	mContext->setTsHandler(h);
}

void WKStreamCtrl::setEventHandler(WKHandler* h)
{
	mContext->setEventHandler(h);
}
*/
bool WKStreamCtrl::connect(const char* url)
{
	// if(mContext->getStreamType() == SK_STREAM_IDLE)
	{
		LOGI("1.0.1 new stream,parse len:%d, url:%s\n", strlen(url), url);
        strErrorUrl.setTo(url);
		
		if(mContext->mMultifailCount < MAX_MULTIFAILED
		)
        {
            LOGD("mul2uni less than 3 times.");
			sk_mc_set_mul2uni(0);
        }
        else
        {
			LOGD("3 times fail multicast , change to unicast channel permanently");
        }

        mUrlParser->parse(url);
		
		if (mUrlParser->mIsChannel)
		{
            LOGD(" play is channel!!!");
			mContext->setStreamType(mContext->getStreamType() | WK_STREAM_CHANNEL);
		}
		else
		{
            LOGD(" play is not channel!!!");
			mContext->setStreamType(mContext->getStreamType() &(~WK_STREAM_CHANNEL));
		}
	}
	// TODO:åˆæ–—–å‰œ˜èˆ»„æ’å‰›¿˜æ˜è‡¬•æ’­
	if (mUrlParser->isMultiLive())
	{
		if (mUDPCtrl->create(mContext, mUrlParser->getMulticastIP(), mUrlParser->getMulticastPort()))
		{
			LOGI("udp ctrl create sucesse");
			mContext->setStreamType(mContext->getStreamType() | WK_STREAM_LIVE_MULTICAST);
			return true;
		}
		else
		{
			LOGE("udp ctrl create error");
			return false;
		}
		// }else if(!mRtspCtrl->connect())
		// return false;
	}
	else
	{
		mContext->mUrl.setTo(mUrlParser->getLiveRtspUrl());
		LOGD("1.0.1 connect %s", mUrlParser->getLiveRtspUrl());
		mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_MULTICAST));
        if (mUrlParser->mIsChannel)
		{
            mContext->setStreamType(mContext->getStreamType() | WK_STREAM_LIVE);
		}
		{
			return mRtspCtrl->connect();
		}
	}
	// return true;
}

bool WKStreamCtrl::play()
{

	int error;
	int len;
	char *p = NULL;
	char infor[1024];
	char range[16] = {0};
	LOGD("streamtype %x &WK_STREAM_MULTICAST %x", mContext->getStreamType(), mContext->getStreamType() & WK_STREAM_MULTICAST);
	WKHandler* h = mContext->mEventHandler;

    memset(range, 0x0, sizeof(range));
    snprintf(range, sizeof(range), "%s", "now");
	if ((mContext->getStreamType() & WK_STREAM_CHANNEL) > 0)
	{
		if ((mContext->getStreamType() & WK_STREAM_LIVE) > 0)
		{
            memset(range, 0x0, sizeof(range));
			snprintf(range, sizeof(range), "%s", "end");
			h->sendMessage(h->obainMessage(EVENT_MEDIA_LIVE));

            h->sendMessage(h->obainMessage(WK_PLAYER_REFRESH_DURATION, 0, mUrlParser->getTimeShiftLength()));
		}
		else
		{
			h->sendMessage(h->obainMessage(EVENT_MEDIA_SHIFT));
		}
	}
	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
	{
		if (mUDPCtrl->joinMulticast(&error))
		{
			LOGI("joind multicast sucess");            
			h->sendMessage(h->obainMessage(EVENT_PLAYMODE_CHANGE, 2, 1));
			return true;
		}
		else
		{

		}

	}
	if (mUrlParser->mIsBookmark)
	{
		WKHandler* h = mContext->mEventHandler;
		h->sendMessage(h->obainMessage(WK_PLAYER_REFRESH_BOOKMARK, atoi(mUrlParser->getBookmark()), 0));
		//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PLAY, 1, 0, strdup(mUrlParser->getBookmark())));
		mRtspCtrl->play(1, 0, strdup(mUrlParser->getBookmark()));
		mUrlParser->mIsBookmark = false;
	}
	else
	{
		//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PLAY, 1, 0, strdup(range)));
		WKMediaProcessor* processor = sk_mc_get_processor();


		mRtspCtrl->play(1, 0, strdup(range));

	}
	// return mRtspCtrl->play(1,"now");
	return true;
}

void WKStreamCtrl::stoping()
{
	mUDPCtrl->stoping();
}

bool WKStreamCtrl::stop()
{
	// if(mUrlParser->isMultiLive())
	// {
	// 	mUDPCtrl->leaveMulticast();
	// }else{
	// 	mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_STOP));
	// 	// mRtspCtrl->stop();
	// 	// mContext->sessionStop();

	// }

	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)	
	{
		mUDPCtrl->leaveMulticast();
	}
	else
	{
		//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_STOP));
		mUDPCtrl->stop();
		if(mRtspCtrl!=NULL)
		{
			mRtspCtrl->stop();
			LOGE("WKStreamCtrl::stop##############################HZY");
			delete mRtspCtrl;
			mRtspCtrl = new WKRtspCtrl();
			mContext->setRtspHandler(mRtspCtrl);
			mRtspCtrl->setContext(mContext);
		    mRtspCtrl->setStream(this);
			mRtspCtrl->setUDPCtrl(mUDPCtrl);
            mUDPCtrl->setRtspCtrl(mRtspCtrl);
		}
	}

    mContext->setStreamType(WK_STREAM_IDLE);
	return true;
}

bool WKStreamCtrl::pause()
{

	// if(mContext->getStreamType() == SK_STREAM_LIVE_MULTICAST)
	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
	{
		mUDPCtrl->leaveMulticast();

		//multicast 2 unicast
		WKMediaProcessor* processor = sk_mc_get_processor();
		//processor->restart();
		processor->seek();

        WKHandler* h = mContext->mEventHandler;
#if 0
        h->sendMessage(h->obainMessage(EVENT_MEDIA_CLEARBUF));
#endif
		mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_MULTICAST) & (~WK_STREAM_LIVE));
		// é€šçŸä»¿â–³éï¼º¿›å…äº¤—å—¥å®
		h->sendMessage(h->obainMessage(EVENT_MEDIA_SHIFT));
		const char *_shifurl = mUrlParser->getShiftUrl();
		mContext->mUrl.setTo(_shifurl);
		if (mRtspCtrl->connect())
		{
			LOGI("multicast to rtsp pause sucesses!");
            //arg1 == 1, send play first.
			//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PAUSE, 1, 0));
			mRtspCtrl->play(1, 0, strdup("end"));
			mRtspCtrl->pause();
			return true;
		}
		else
		{
			LOGE("multicast to rtsp error,rtsp ctrl connect faild!");
			return false;
		}
	}
	if (((mContext->getStreamType() & WK_STREAM_CHANNEL) > 0) && ((mContext->getStreamType() & WK_STREAM_LIVE) > 0))
	{
		mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_LIVE));
		// é€šçŸä»¿â–³éï¼º¿›å…äº¤—å—¥å®
		WKHandler* h = mContext->mEventHandler;
		h->sendMessage(h->obainMessage(EVENT_MEDIA_SHIFT));
		
	}
	// æº–éƒªç•¦è¢¨æ€“ç‹Ÿ,é£²èŒ¼è·¤rtspæ¥·å†žpauseç¡Œé”
	//else
	//{
	//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PAUSE));
	{
		mRtspCtrl->pause();
	}
	//}
	// return mRtspCtrl->pause();
	return true;
}

bool WKStreamCtrl::trick(int scale)
{

	// if(mContext->getStreamType() == SK_STREAM_LIVE_MULTICAST)
	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
	{
		if (scale > 0)
		{
			LOGE("live state cannot trick forward!");
			WKHandler* h = mContext->mEventHandler;
			h->sendMessage(h->obainMessage(EVENT_MEDIA_END, 1, 0));
			return true;
		}
		else
		{
			LOGI("multicast to rtsp sucesses!");
			mUDPCtrl->leaveMulticast();

            mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_MULTICAST) & (~WK_STREAM_LIVE));
            
			//multicast 2 unicast
			WKMediaProcessor* processor = sk_mc_get_processor();
			//processor->restart();
			processor->seek();
		
			// é€šçŸä»¿â–³éï¼º¿›å…äº¤—å—¥å®
			WKHandler* h = mContext->mEventHandler;
			h->sendMessage(h->obainMessage(EVENT_MEDIA_SHIFT));
			mContext->mUrl.setTo(mUrlParser->getShiftUrl());
			if (mRtspCtrl->connect())
			{
				LOGI("multicast to rtsp sucesses!--3");
				//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PLAY, scale, 0, strdup("now")));
				mRtspCtrl->play(scale, 0, strdup("now"));
				return true;
			}
			else
			{
				LOGE("multicast to rtsp error,rtsp ctrl connect faild!");
				return false;
			}
		}
	}
    
	if (((mContext->getStreamType() & WK_STREAM_LIVE) > 0) && ((mContext->getStreamType() & WK_STREAM_CHANNEL) > 0))
	{
		if (scale > 0)
		{
			LOGE("live state cannot trick forward!");
			WKHandler* h = mContext->mEventHandler;
			h->sendMessage(h->obainMessage(EVENT_MEDIA_END, 1, 0));
			return true;
		}
		else
		{
			mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_LIVE));
			// é€šçŸä»¿â–³éï¼º¿›å…äº¤—å—¥å®
			WKHandler* h = mContext->mEventHandler;
			h->sendMessage(h->obainMessage(EVENT_MEDIA_SHIFT));

		}
	}
	//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PLAY, scale, 0, strdup("now")));
	mRtspCtrl->play(scale, 0, strdup("now"));
	// return mRtspCtrl->play(scale,"now");
	return true;
}
bool WKStreamCtrl::seekTo(int mode, const char* t)
{    
	// if(mContext->getStreamType() == SK_STREAM_LIVE_MULTICAST)
	LOGD("streamtype %x &WK_STREAM_CHANNEL %x", mContext->getStreamType(), mContext->getStreamType() & WK_STREAM_CHANNEL);
    WKMediaProcessor* processor = sk_mc_get_processor();
    if (((mContext->getStreamType() & WK_STREAM_CHANNEL) > 0 ))
	{
        if(mode == 4)
        {
		    mContext->setStreamType(mContext->getStreamType() | (WK_STREAM_LIVE));
        }
        else
        {
            mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_LIVE));
        }
	}
	WKHandler* h = mContext->mEventHandler;
	if ((mContext->getStreamType()&WK_STREAM_CHANNEL) > 0)
	{
		if ((mContext->getStreamType() & WK_STREAM_LIVE) > 0)
			h->sendMessage(h->obainMessage(EVENT_MEDIA_LIVE));
		else
			h->sendMessage(h->obainMessage(EVENT_MEDIA_SHIFT));
	}
    
	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
	{
		mUDPCtrl->leaveMulticast();
		//multicast 2 unicast
		//processor->restart();
		processor->seek();
		mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_MULTICAST) & (~WK_STREAM_LIVE));
		const char *timeShiftUrl = mUrlParser->getShiftUrl();
		mContext->mUrl.setTo(timeShiftUrl);

		LOGI("[%s:%d] live to timeshift, timeshiftUrl:%s ", __FUNCTION__, __LINE__, timeShiftUrl);
		if (mRtspCtrl->connect())
		{
			LOGI("multicast to rtsp sucesses!-----1");
			//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PLAY, 1, mode, strdup(t)));
			mRtspCtrl->play(1, mode, strdup(t));
			return true;
		}
		else
		{
			LOGE("multicast to rtsp error,rtsp ctrl connect faild!");
			return false;
		}
	}


	//mRtspCtrl->sendMessage(mRtspCtrl->obainMessage(SK_RTSP_CTRL_PLAY, 1, mode, strdup(t)));

    //processor->seek();
    mRtspCtrl->play(1, mode, strdup(t));
	
	// return mRtspCtrl->play(1,t);
	return true;
}

/*
bool WKStreamCtrl::isChannel()
{
	// if(mRtspCtrl->getCurrentStreamType() == SK_STREAM_MODE_VOD)
	if ((mContext->getStreamType() & WK_STREAM_CHANNEL) == 0)
		return false;
    
	return true;
}
bool WKStreamCtrl::isLive()
{	
    if ((mContext->getStreamType() & WK_STREAM_LIVE) == 0)
		return false;
    
	return true;
}
void WKStreamCtrl::ctlBufferState(int burstMode)
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
*/
bool WKStreamCtrl::mul2uni()
{
	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
	{
		mUDPCtrl->leaveMulticast();

		//multicast 2 unicast
		WKMediaProcessor* processor = sk_mc_get_processor();
		//processor->restart();
		processor->seek();
		
		mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_MULTICAST));
		mContext->setStreamType(mContext->getStreamType() | WK_STREAM_LIVE);

        const char* uniUrl = mUrlParser->getLiveRtspUrl();

        if(strlen(uniUrl) <= 0)
        {
            mContext->mMul2UniIsFail = true;
            mContext->mEventHandler->sendMessage(mContext->mEventHandler->obainMessage(WK_PLAYER_BACKTOLIVE));
            return true;
        }
        
		mContext->mUrl.setTo(uniUrl);
		LOGI("[%s:%d] uniUrl:%s ", __FUNCTION__, __LINE__, uniUrl);
		if (mRtspCtrl->connect())
		{
			LOGI("multicast to rtsp sucesses!--2");
			if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
	        	{
		                int error;
		                mContext->mMul2UniIsFail = true;
	        		if (mUDPCtrl->joinMulticast(&error))
	        		{
	        			LOGI("joind multicast sucess");
	        			return true;
	        		}
	        	}
	                else
	                {
				mRtspCtrl->play(1, 0, strdup("end"));
	                }
			
			return true;
		}
		else
		{
			LOGE("multicast to rtsp error,rtsp ctrl connect faild!");
			return false;
		}
	}
	return true;
}

void WKStreamCtrl::releaseRtspSingle(bool isStop)
{
	if(mRtspCtrl != NULL)
	    mRtspCtrl->releaseSingle(isStop);
}
/*

int WKStreamCtrl::getTimeshiftLength()
{
#if 0
    time_t now;
    time(&now);

    now -= mUrlParser->getTimeShiftLength();
    return now;
#else
    if(mUrlParser == NULL)
        return 0;

    return mUrlParser->getTimeShiftLength();
#endif
}

bool WKStreamCtrl::isMultiLive()
{   
    if(mUrlParser == NULL)
        return false;

    return mUrlParser->isMultiLive();
}

bool WKStreamCtrl::trickAble()
{
    bool ret = false;
    
    if(mUrlParser != NULL)
        ret = mUrlParser->mTrickable;

    return ret;
}



int WKStreamCtrl::getCurrentProtocolType()
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
*/
