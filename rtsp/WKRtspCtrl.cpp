#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "WKRtspCtrl.h"
#include "WKRtsp.h"
#include "WKMessage.h"
#include "WKMessageQueue.h"
#include "WKLooper.h"
#include "WKSessionContext.h"
#include "WKRtspCmd.h"
#include "WKURLParser.h"
#include "WKRtspRequest.h"
#include "wk_android_std.h"
#include "WKRtspResponseDecoder.h"
#include "WKUDP.h"
#include "WKStreamCtrl.h"

#include "WKNetUtils.h"
#include "WKMediaProcessor.h"
#include <String.h>
#include <stdlib.h>
#include <string.h>


#define MODULE_NAME "SKIPTV_RTSP_CTRL"



#define WK_RTSP_HEART_TIME 10000


#define WK_RTSP_UA "CTC RTSP 1.0"

#define TRANS_STR_TCP "MP2T/TCP;unicast;destination=%s;interleaved=0-1;mode=PLAY"
#define TRANS_STR_RTP_TCP "MP2T/RTP/TCP;unicast;destination=%s;interleaved=0-1;mode=PLAY"
#define TRANS_STR_UDP "MP2T/UDP;unicast;destination=%s;client_port=%u"
#define TRANS_STR_RTP_UDP "MP2T/RTP/UDP;unicast;destination=%s;client_port=%u"

#define TRANS_STR_TCP2 "MP2T/TCP;unicast;client_address=%s;interleaved=0-1;mode=PLAY"
#define TRANS_STR_RTP_TCP2 "MP2T/RTP/TCP;unicast;client_address=%s;interleaved=0-1;mode=PLAY"
#define TRANS_STR_UDP2 "MP2T/UDP;unicast;client_address=%s;dest_port=%u"
#define TRANS_STR_RTP_UDP2 "MP2T/RTP/UDP;unicast;client_address=%s;dest_port=%u"

#define GET_TRANS_STR(a, b, c, d) "Transport: "a","b","c","d"\r\n"

using namespace android;

WKRtspCtrl::WKRtspCtrl()
{
	//construct
	// mRtsp = NULL;
	mLastScale = 1;
	mRtsp = new WKRtsp();
	mContext = NULL;
	mRequest = NULL;
    mStream = NULL;
	mLooper = new WKLooper();
	mCseq = 1;
    mPlayMode = 0;
    mHearBeatStarted = false;
    mIsStopping = false;
    mIsZteServ = false;
	mState = WK_RTSP_CTRL_STATE_IDLE;
	// mStreamType = WK_STREAM_MODE_NULL;
	if (mLooper->loop())
	{
		setLooper(mLooper);
		LOGI("WKRtspCtrl create looper success!");
	}
	else
		LOGE("WKRtspCtrl create looper falid!");

	//get the random port
	mUDPPort = getRandomPort();
    mTransProtocol = sk_mc_get_rtsp_protocal();

}


WKRtspCtrl::~WKRtspCtrl()
{
	//destruct
	LOGE("SKRTSP Ctrl destroy"); 

	if (mLooper != NULL)
	{
		sendMessage(WKMessage::obain(NULL));
	}
	mRtsp = NULL;
	mRequest = NULL;

	//:TODO should wait for looper thread exit??
	mLooper = NULL;
}

void WKRtspCtrl::dispatchMessage(WKMessage* msg)
{
	//
	switch (msg->what)
	{
	case WK_RTSP_CTRL_TIMEOUT:
	{
		WKRtspRequest* req = findRequest(msg->arg1);
        mSignal.broadcast();
		if (req != NULL)
		{
            LOGE("command timeout :%d", msg->arg1);
            //if(req->getRequest() == STOP || req->getRequest() == PLAY || req->getRequest() == PAUSE)
            {    
                LOGD("command[%d] timeout, release lock!!", req->getRequest());
            }
            /*else
            {
			    sendMessage(obainMessage(SK_RTSP_CTRL_ERROR));
			    sp<SKHandler> h = mContext->mEventHandler;
			    h->sendMessage(h->obainMessage(EVENT_MEDIA_ERROR, RTSP_READ_FAILED, 0));
            }*/
		}
		break;
	}
	case WK_RTSP_CTRL_RESPONSE:
	{
        //remove timeout message.
        (mLooper->getMessageQueue())->removeMessages(WK_RTSP_CTRL_TIMEOUT);
        
		WKRtspResponseDecoder* pr = static_cast<WKRtspResponseDecoder*>(msg->obj);
		WKRtspResponseDecoder* r = pr;

        if(!mIsZteServ && (NULL != r->server) && (NULL != strstr(r->server, "ZX")))
        {
            mIsZteServ = true;
        }
        
		// if((mState & WK_RTSP_CTRL_STATE_CONNECTED) !=WK_RTSP_CTRL_STATE_CONNECTED)
		if (mState == WK_RTSP_CTRL_STATE_STOPPED || mState == WK_RTSP_CTRL_STATE_ERROR)
		{
			LOGI("already stoped,or error");
            mSignal.broadcast();
			break;
		}
		WKRtspRequest* req = findRequest(r->cseq);
		LOGD("recive respone %d", r->cseq);
		if (req == NULL)
		{
			LOGE("bad response,can not find request");
            mSignal.broadcast();
			// sendMessage(obainMessage(SK_RTSP_CTRL_ERROR));
			// 可能是之前的节目遗留下来的响应
			break;
		}
		switch (req->getRequest())
		{
		case DESCRIBE:
		{
			if (strncmp(r->retcode, "200", 3) == 0)
			{
				WKHandler* h = mContext->mEventHandler;
				LOGI("DESCRIBE mRtsp->mUrl:%s",mRtsp->mUrl);
				if ((r->mStreamType) == "live")
				{
					//mStreamType = SK_STREAM_MODE_LIVE;
					WKHandler* tsh = mContext->mTsHandler;
					//tsh->sendMessage(tsh->obainMessage(SK_TS_NO_PTS));
				}
				else if ((r->mStreamType) == "vod")
				{
					//mStreamType = SK_STREAM_MODE_VOD;	
				}
	
			mVodEndTime = r->rangeEnd;

 
               if (((mContext->getStreamType() & WK_STREAM_CHANNEL) > 0) && ((mContext->getStreamType() & WK_STREAM_LIVE) > 0))
 
               {
 
               }
 
               else
 
               {
	
			    h->sendMessage(h->obainMessage(WK_PLAYER_REFRESH_DURATION, r->rangeStart, r->rangeEnd));
 
               }
 
               
 
               if ((mContext->getStreamType() & WK_STREAM_CHANNEL) && (mContext->getStreamType() & WK_STREAM_LIVE)
 
                   && r->sdp_connect_addr != NULL && r->sdp_media_port != 0
 
                   && !mContext->mHasMul2Uni)
 
               {
 
                   unsigned long recv_ip = inet_addr(r->sdp_connect_addr);

 
                   if(IS_MULTICAST_IP(recv_ip))
 
                   {
 
                       mRtsp->stop();
 
                       mContext->setStreamType(mContext->getStreamType() | WK_STREAM_LIVE_MULTICAST); 
                       mUDPCtrl->create(mContext, recv_ip, (unsigned short)r->sdp_media_port);
 
                       mState = WK_RTSP_CTRL_STATE_CONNECTED;
 
                       mSignal.broadcast();
 
                       break;
 
                   }
 
               }
	
			sendMessage(obainMessage(WK_RTSP_CTRL_SETUP));
	
		
	
		}
	
		else if (r->retcode[0] == '3' && NULL != r->location)	// 重定向处理
	
		{
	
			LOGI("redirect->location:%s", r->location);
	
			mContext->mUrl.setTo(r->location);
	
			sendMessage(obainMessage(WK_RTSP_CTRL_DESCRIBE));
	
		}
 
           else
	
		{
	
			sendMessage(obainMessage(WK_RTSP_CTRL_ERROR));
	
		}
	
		break;
	
	}
	
	case SETUP:
	
	{
	
		if (strncmp(r->retcode, "200", 3) == 0)
	
		{
 
               mState = WK_RTSP_CTRL_STATE_CONNECTED;
	
			if(r->session != NULL)
	
	        {
	
	    		mSession.setTo(r->session);
	
	        }
	
			if (r->transport)
	
			{
	
				if (strstr(r->transport, "UDP") > 0)
	
				{
	
					LOGI("rtsp transport rtp-udp");
	
					mContext->setStreamType(mContext->getStreamType() | WK_STREAM_UDP);
	
					if (!mUDPCtrl->uniPlay(mContext, 0, mUDPPort))
	
					{
	
						LOGE("create uni udp socket faild!");
	
					}
	
				}
	
				else if (strstr(r->transport, "TCP") > 0)
	
				{
	
					mContext->setStreamType(mContext->getStreamType() & (~WK_STREAM_UDP));
	
				}
	
				else
	
				{
	
					LOGE("unknow transport protocal!");
	
				}

 
                   if(strstr(r->transport, "RTP") > 0)
 
                   {
 
                       mContext->setHasRtp(true);
 
                   }
	
			}
	
			else
	
			{
	
				LOGE("can not found transport protocal!");
	
			}
	
			mSignal.broadcast();	
		}
	
		else if (r->retcode[0] == '3' && NULL != r->location)	// 重定向处理
	
		{
	
			LOGI("redirect->location:%s", r->location);
	
			mContext->mUrl.setTo(r->location);
	
			sendMessage(obainMessage(WK_RTSP_CTRL_DESCRIBE));
	
		}
 
           else			// other code,some like error
	
		{
	
			sendMessage(obainMessage(WK_RTSP_CTRL_ERROR));
	
		}
	
		break;
	
	}
	
	case PLAY:
	
	{
	
		if (strncmp(r->retcode, "200", 3) == 0 || strncmp(r->retcode, "457", 3) == 0)
	
		{
 
               if(strncmp(r->retcode, "200", 3) == 0)
 
               {
	
			    LOGI("200 PLAY redirect->rtp_info:%s",r->rtp_info);
 
               }
 
               else
 
               {
 
                   LOGE("457 Invalid Range");
 
               }
	
			LOGD("........r->range:%s", r->range);
	
			LOGD("........r->scale:%s", r->scale);
	
			WKHandler* h = mContext->mEventHandler;	
			if (r->scale==NULL || strncasecmp(r->scale, "1.0", strlen("1.0")) == 0)
	
			{
	
				LOGD("send EVENT_PLAYMODE_CHANGE");
	
				mState = WK_RTSP_CTRL_STATE_PLAYING;

	
				h->sendMessage(h->obainMessage(EVENT_PLAYMODE_CHANGE, 2, 1));

	
			}
	
			else
	
			{
	
				mState = WK_RTSP_CTRL_STATE_TRICKING;
	
				h->sendMessage(h->obainMessage(EVENT_PLAYMODE_CHANGE, 3, atoi(r->scale)));	
			} 
               if(!mHearBeatStarted)
 
               {
	
			    mHearBeatStarted = true;

 
   				// play完之后立即发
 
   				sendMessage(obainMessage(WK_RTSP_CTRL_GETPARAMETER), uptimeMillis() + WK_RTSP_HEART_TIME);

 
               }	
            mSignal.broadcast();
	
		}
	
		else if(r->retcode[0] == '3' && NULL != r->location)	// PLAY 重定向处理
	
		{
			mContext->mUrl.setTo(r->location);

				sendMessage(obainMessage(WK_RTSP_CTRL_PLAY, 2, 1, NULL));

			}
			else
			{
				sendMessage(obainMessage(WK_RTSP_CTRL_ERROR));
			}
			break;
		}
		case PAUSE:
		{
            mSignal.broadcast();
			if (strncmp(r->retcode, "200", 3) == 0)
			{
				LOGD("recive pause response,send modechange");
				mState = WK_RTSP_CTRL_STATE_PAUSEING;
				WKHandler* h = mContext->mEventHandler;
				h->sendMessage(h->obainMessage(EVENT_PLAYMODE_CHANGE, 1, 0));

                if(!mHearBeatStarted)
                {
				    mHearBeatStarted = true;
                    sendMessage(obainMessage(WK_RTSP_CTRL_GETPARAMETER), uptimeMillis() + WK_RTSP_HEART_TIME);
                }
			}
			break;
		}
        case STOP:
        {
			LOGD("GET RESPONSE STOP");
#if 0
            LOGD("GET RESPONSE STOP, now broadcast!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            mSignal.broadcast();
#endif
            break;
        }
		case GETPARAMETER:
		{
				if (r->time_current)
				{
					char* t = strdup(r->time_current);
					WKHandler* h = mContext->mEventHandler;
					h->sendMessage(h->obainMessage(WK_PLAYER_REFRESH_TIME, 0, 0, t));
				}
                sendMessage(obainMessage(WK_RTSP_CTRL_GETPARAMETER), uptimeMillis() + WK_RTSP_HEART_TIME);
             
			break;
		}
		case GETPARAMETER_BURST_OPEN:
		{
			// do nothing;
			break;
		}
		case GETPARAMETER_BURST_CLOSE:
		{
			break;
		}
		default:
			LOGI("bad request");
			break;
		}
		break;
	}
	case WK_RTSP_CTRL_ANNOUNCE:
	{
		LOGD("TODO:deal with ANNOUNCE");
		char *temp = (char*)(msg->obj);
		if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) == WK_RTSP_CTRL_STATE_CONNECTED)
		{
			char *p = strstr(temp, "x-notice:");
			if (p == 0)
			{
				free(temp);
				break;
			}
			p += strlen("x-notice:");
			while (isspace(*p) && (*p != '\0'))p++;

			LOGE("%s", p);
			if (strncasecmp(p, "2101", 4) == 0)
			{
				WKHandler* h = mContext->mEventHandler;
				// if(mStreamType == SK_STREAM_MODE_LIVE || mStreamType == SK_STREAM_MODE_SHIFT)
				if ((mContext->getStreamType() & WK_STREAM_CHANNEL) > 0)
				{
					mContext->setStreamType(mContext->getStreamType() | WK_STREAM_LIVE);
					LOGD("stream end ,channel,%x", mContext->getStreamType()& WK_STREAM_LIVE);
					h->sendMessage(h->obainMessage(EVENT_MEDIA_END, 1, 0));
				}
				else
				{
					WKMediaProcessor* processor = sk_mc_get_processor();
					processor->setEOF();
					//h->sendMessage(h->obainMessage(EVENT_MEDIA_END));
				}
			}
			else if (strncasecmp(p, "2102", 4) == 0)
			{
				WKHandler* h = mContext->mEventHandler;
				// if(mStreamType == SK_STREAM_MODE_LIVE || mStreamType == SK_STREAM_MODE_SHIFT)
				if ((mContext->getStreamType() & WK_STREAM_CHANNEL) > 0)
				{
					h->sendMessage(h->obainMessage(EVENT_MEDIA_BEGINING, 1, 0));
				}
                else
                {
					// :for taizhou
					// h->sendMessage(h->obainMessage(EVENT_MEDIA_BEGINING));
					h->sendMessage(h->obainMessage(EVENT_MEDIA_BEGINING, 1, 0));
                }
				// :end
			}
			else if (strncasecmp(p, "2103", 4) == 0)
			{
				LOGD("Annouce 2103: Force Exit \n");
//				WKHandler* h = mContext->mEventHandler;
//				h->sendMessage(h->obainMessage(EVENT_MEDIA_ERROR, 1, 0));
			}
		}
        else
        {
            mSignal.broadcast();
        }
		free(temp);
		break;
	}
	case WK_RTSP_CTRL_DESCRIBE:
	{
		LOGD("do describe");
		// if(mState == WK_RTSP_CTRL_STATE_IDLE || mState == WK_RTSP_CTRL_STATE_STOPPED || mState == WK_RTSP_CTRL_STATE_CONNECTING)
		// {
		mState = WK_RTSP_CTRL_STATE_CONNECTING;
		if (!describe())
		{
			LOGE("rtsp ctrl send describe error");
			WKMessage* msg = obainMessage(WK_RTSP_CTRL_ERROR);
			sendMessage(msg);
		}
		// }else{
		//     LOGD("call rtsp describe on wrong state,now:%d",mState);
		// }
		break;
	}
	case WK_RTSP_CTRL_SETUP:
	{
		LOGD("do setup");
		if (!setup())
		{
			LOGE("rtsp ctrl send setup error");
			WKMessage* msg = obainMessage(WK_RTSP_CTRL_ERROR);
			sendMessage(msg);
		}
		break;
	}
	case WK_RTSP_CTRL_PLAY:
	{
		LOGD("do play");
		char* temp = (char*)(msg->obj);
        String t("");
        if(temp != NULL)
        {
    		t.setTo(temp);
    		free(temp);
        }
		if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) == WK_RTSP_CTRL_STATE_CONNECTED)
		{
            //一键到头
			if (msg->arg2 == 3)
			{
                mPlayMode = 3;
				// if(mStreamType == SK_STREAM_MODE_VOD)
                {
					t.setTo("beginning");
                }
			}
            //一键到尾
			else if (msg->arg2 == 4)
			{
                mPlayMode = 4;
				// if(mStreamType == WK_STREAM_MODE_VOD)
				{
					t.setTo("end");
				}

			}
            else
            {
                mPlayMode = 0;
                if ((mContext->getStreamType() & WK_STREAM_CHANNEL))
                {
                    int chanBaseTime = sk_mc_ascutc2time(t.string());
                    WKHandler* h = mContext->mEventHandler;
                    //h->sendMessage(h->obainMessage(WK_PLAYER_REFRESH_BASETIME, chanBaseTime, 0));
                }
            }
			// if(mStreamType == WK_STREAM_MODE_VOD)
			if ((mContext->getStreamType() & WK_STREAM_CHANNEL) == 0)
			{
				if (strncasecmp(t.string(), "now", 3) != 0 
                    && strncasecmp(t.string(), "end", 3) != 0 
                    && strncasecmp(t.string(), "beginning", 9) != 0)
				{
					t.append(".000000");
				}
			}

			if (!doPlay(msg->arg1, t.string()))
			{
				LOGE("rtsp send play command error");
				// 已经连接成功，暂时不比通知错误状态
				// sp<SKMessage> msg = obainMessage(SK_RTSP_CTRL_ERROR);
				// sendMessage(msg);
				mSignal.broadcast();
			}
		}
        else
        {
            mSignal.broadcast();
        }
		break;
	}
	case WK_RTSP_CTRL_GETPARAMETER:
	{

		if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) == WK_RTSP_CTRL_STATE_CONNECTED)
		{
			LOGD("do get parameter");
			if (!getParameter(msg->arg1))
			{
				LOGE("rtsp send get parameter command error");
				// WKMessage* msg = obainMessage(WK_RTSP_CTRL_ERROR);
				// sendMessage(msg);
				WKHandler* h = mContext->mEventHandler;
				h->sendMessage(h->obainMessage(EVENT_MEDIA_ERROR, RTSP_READ_FAILED, 0));
			}
		}
		break;
	}
	case WK_RTSP_CTRL_PAUSE:
	{
		LOGD("do pause");
		if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) == WK_RTSP_CTRL_STATE_CONNECTED)
		{
			if (!doPause())
			{
				LOGE("rtsp send pause command error");
				// WKMessage* msg = obainMessage(WK_RTSP_CTRL_ERROR);
				// sendMessage(msg);
				mSignal.broadcast();
			}
		}
        else
        {
            mSignal.broadcast();
        }
		break;
	}
	case WK_RTSP_CTRL_STOP:
	{
		LOGD("rtsp do stop");
        //if (mState == SK_RTSP_CTRL_STATE_CONNECTING)
		//	mSignal.broadcast();
		// 用消息机制运行，只会在消息处理线程改变mState的状态，不会造成冲突，所以不必加锁
		doStop();
		//Stop do not need wait response
		mSignal.broadcast();
		break;
	}
	// case SK_RTSP_SOCKET_ERROR:
	case WK_RTSP_CTRL_ERROR:
	{
		//RTSP response error code , such as describe setup play.
		LOGE("rtsp ctrl error");
		// TODO:
		
		//if (mState == SK_RTSP_CTRL_STATE_CONNECTING)
			mSignal.broadcast();
		// close connect
        if(mState != WK_RTSP_CTRL_STATE_STOPPED && !mIsStopping)
        {
		    mState = WK_RTSP_CTRL_STATE_ERROR;
            WKHandler* h = mContext->mEventHandler;
		    h->sendMessage(h->obainMessage(EVENT_MEDIA_ERROR, RTSP_READ_FAILED, 0));
        }
		break;
	}
	default:
		LOGI("unkown message:%x", msg->what);
		break;
	}
}

void WKRtspCtrl::setContext(WKSessionContext* context)
{
	mContext = context;
}

void WKRtspCtrl::setUDPCtrl(WKUDP* uc)
{
	mUDPCtrl = uc;
}

void WKRtspCtrl::releaseSingle(bool isStop)
{
    if(isStop)
    {
        mIsStopping = true;
    }
    
    mSignal.broadcast();
}

bool WKRtspCtrl::connect()
{
    int reTimes = 0;
    int reCount = 3;
    bool ret = false;
    
//	LOGD("rtsp connect require lock");
	AutoMutex _l(mLock);
    mState = WK_RTSP_CTRL_STATE_IDLE;
    
    while(reTimes < reCount)
    {
//	    LOGD("rtsp connect get lock");
    	WKMessage* msg = obainMessage(WK_RTSP_CTRL_DESCRIBE);
    	sendMessage(msg);
    	
    	LOGD("connect wait lock...");
        mSignal.wait(mLock);
    	//mSignal.waitRelative(mLock, uptimeMillis() + 2000);
    	LOGD("connect get lock");

//      if(mSignal.waitRelative(mLock, milliseconds(5000)) != 0)
//		LOGE("connect timeout");

    	LOGD("mstate:%x,%x,%x", mState, WK_RTSP_CTRL_STATE_CONNECTED, mState & WK_RTSP_CTRL_STATE_CONNECTED);

        if(mState == WK_RTSP_CTRL_STATE_STOPPED || mIsStopping)
        {
            LOGD("rtsp is stopped, exit connect now.");
            ret = true;
            break;
        }

        ret = ((mState & WK_RTSP_CTRL_STATE_CONNECTED) == WK_RTSP_CTRL_STATE_CONNECTED) ? true : false;

        if(ret)
        {
            break;
        }
        else
        {
            reTimes++;
        }
    }

	if(!ret)
	{
		WKHandler* h = mContext->mEventHandler;
		h->sendMessage(h->obainMessage(EVENT_MEDIA_ERROR, RTSP_CONNECT_FAILED, 0));
	}
	else
	{
	}
		
	return ret;
}

bool WKRtspCtrl::describe()
{
	mCseq = 1;		// 重定向后要从1开始
	mHearBeatStarted = false;
    mIsStopping = false;
    mIsZteServ = false;
	int errr, len;
	char *p = NULL;
	char infor[1024];
	if (mRtsp->rtsp_create_client(mContext, &errr))
	{
		String cmdbuf;

		//cmdbuf.appendFormat("DESCRIBE %s RTSP/1.0\r\n",mContext->mUrl.string());
        //cmdbuf.appendFormat("DESCRIBE %s&hms_devid=4&prioritypopid=2 RTSP/1.0\r\n", mRtsp->mUrl);
		cmdbuf.appendFormat("DESCRIBE %s RTSP/1.0\r\n", mRtsp->mUrl);
		cmdbuf.appendFormat("CSeq: %u\r\n", mCseq);
		cmdbuf.append("Accept: application/sdp \r\n");
        cmdbuf.append("User-Agent: "WK_RTSP_UA"\r\n");

		cmdbuf.append("x-Burst\r\n");
		cmdbuf.append("\r\n");

		if (!mRtsp->sendCommand(cmdbuf.string()))
		{
			return false;
		}
		// LOGD("describe require mlock");
		appendRequest(new WKRtspRequest(mCseq++, DESCRIBE));
		// LOGD("describe get mlock");
		sendMessage(obainMessage(WK_RTSP_CTRL_TIMEOUT, mCseq - 1, 0), uptimeMillis() + 10000);  //3000
		return true;
	}
	else
	{
        if(mState != WK_RTSP_CTRL_STATE_STOPPED && !mIsStopping)
        {
		    WKHandler* h = mContext->mEventHandler;
		    h->sendMessage(h->obainMessage(EVENT_MEDIA_ERROR, RTSP_CONNECT_FAILED, 0));
        }

		LOGE("create rtsp client error");
		return false;
	}
}
bool WKRtspCtrl::setup()
{
	String cmdbuf;
    const char *localip ;
	localip = mUDPCtrl->getLocalIPString();
	char lan_ip[64] = {0};
	strncpy(lan_ip, localip, 16);

    // cmdbuf.appendFormat("SETUP %s RTSP/1.0\r\n",mContext->mUrl.string());
//	cmdbuf.appendFormat("SETUP %s&hms_devid=4&prioritypopid=2 RTSP/1.0\r\n", mRtsp->mUrl);
	cmdbuf.appendFormat("SETUP %s RTSP/1.0\r\n", mRtsp->mUrl);
	cmdbuf.appendFormat("CSeq: %u\r\n", mCseq);
	
    LOGD("RTSP SETUP, mTransProtocol = %d, lan_ip:[%s], udp_port:%u", mTransProtocol, lan_ip, mUDPPort);
	switch(mTransProtocol)
	{
		case WK_RTSP_TRANS_TCP:

		    {
			    cmdbuf.appendFormat(GET_TRANS_STR(TRANS_STR_TCP, TRANS_STR_RTP_TCP, TRANS_STR_RTP_UDP, TRANS_STR_UDP), lan_ip, lan_ip, lan_ip, mUDPPort, lan_ip, mUDPPort);
		    }

			break;
		case WK_RTSP_TRANS_RTP_TCP:
			cmdbuf.appendFormat(GET_TRANS_STR(TRANS_STR_RTP_TCP, TRANS_STR_TCP, TRANS_STR_RTP_UDP, TRANS_STR_UDP), lan_ip, lan_ip, lan_ip, mUDPPort, lan_ip, mUDPPort);
			break;
		case WK_RTSP_TRANS_UDP:
			cmdbuf.appendFormat(GET_TRANS_STR(TRANS_STR_UDP, TRANS_STR_RTP_UDP, TRANS_STR_TCP, TRANS_STR_RTP_TCP), lan_ip, mUDPPort, lan_ip, mUDPPort, lan_ip, lan_ip);
			break;
		case WK_RTSP_TRANS_RTP_UDP:
			cmdbuf.appendFormat(GET_TRANS_STR(TRANS_STR_RTP_UDP, TRANS_STR_UDP, TRANS_STR_TCP, TRANS_STR_RTP_TCP), lan_ip, mUDPPort, lan_ip, mUDPPort, lan_ip, lan_ip);
			break;
		default:
			cmdbuf.appendFormat(GET_TRANS_STR(TRANS_STR_TCP, TRANS_STR_RTP_TCP, TRANS_STR_RTP_UDP, TRANS_STR_UDP), lan_ip, lan_ip, lan_ip, mUDPPort, lan_ip, mUDPPort);
	        break;
	}
    
	cmdbuf.append("User-Agent: "WK_RTSP_UA"\r\n");
	cmdbuf.append("\r\n");

	if (!mRtsp->sendCommand(cmdbuf.string()))
	{
		return false;
	}
	appendRequest(new WKRtspRequest(mCseq++, SETUP));
	// sendMessage(obainMessage(WK_RTSP_CTRL_TIMEOUT,mCseq-1,0));
	sendMessage(obainMessage(WK_RTSP_CTRL_TIMEOUT, mCseq - 1, 0), uptimeMillis() + 10000);
	return true;
}
bool WKRtspCtrl::play(float scale, int mode, const char* range)
{
    if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) != WK_RTSP_CTRL_STATE_CONNECTED)
    {
        return false;
    }

	mLastScale = (int)scale;

    AutoMutex _l(mLock);

    WKMessage* msg = obainMessage(WK_RTSP_CTRL_PLAY, scale, mode, (void*)range);
	sendMessage(msg);
#if 0
    LOGD("play wait lock...");
    mSignal.wait(mLock);
    LOGD("play get lock...");
#endif
    if(mSignal.waitRelative(mLock, milliseconds(2000)) != 0)
		LOGE("play timeout");
    return true;
}
bool WKRtspCtrl::doPlay(float scale, const char* range)
{
	String cmdbuf;
	// cmdbuf.appendFormat("PLAY %s RTSP/1.0\r\n",mContext->mUrl.string());
	char host[2048];
	char *host_p = NULL;
	char *p = NULL;
	int len;
	char infor[1024];
	
	mLastScale = (int)scale;
	mLastPlayRange.setTo(range);

//	cmdbuf.appendFormat("PLAY %s&hms_devid=4&prioritypopid=2 RTSP/1.0\r\n", mRtsp->mUrl);
	cmdbuf.appendFormat("PLAY %s RTSP/1.0\r\n", mRtsp->mUrl);
	cmdbuf.appendFormat("CSeq: %u\r\n", mCseq);
	LOGI("[doPlay] mRtsp->mUrl=%s", mRtsp->mUrl);
	if ((strchr(range, 'Z') == 0) && (strchr(range, 'z') == 0))
	{
        if(strcmp(range, "") == 0)
        {
            cmdbuf.appendFormat("Range: npt=%s-\r\n", "now");
        }
        else
        {
            cmdbuf.appendFormat("Range: npt=%s-\r\n", range);
        }
	}
	else
	{
		cmdbuf.appendFormat("Range: clock=%s-\r\n", range);
	}

		cmdbuf.appendFormat("Scale: %.1f\r\n", scale);

	cmdbuf.appendFormat("Session: %s\r\n", mSession.string());
	cmdbuf.append("User-Agent: "WK_RTSP_UA"\r\n");
	cmdbuf.append("\r\n");

   
	if (!mRtsp->sendCommand(cmdbuf.string()))
	{
		return false;
	}
	
	appendRequest(new WKRtspRequest(mCseq++, PLAY));
    sendMessage(obainMessage(WK_RTSP_CTRL_TIMEOUT, mCseq - 1, 0), uptimeMillis() + 3000);
	return true;
}

bool WKRtspCtrl::stop()
{
	LOGE("WKRtspCtrl stop");
    if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) != WK_RTSP_CTRL_STATE_CONNECTED)
    {
		mRtsp->stop();
        return false;
    }
	String cmdbuf;
    AutoMutex _l(mLock);
	WKMessage* msg = obainMessage(WK_RTSP_CTRL_STOP);
	sendMessage(msg);
	//To avoid Dead Lock , timeout is 2 seconds
    if(mSignal.waitRelative(mLock, milliseconds(2000)) != 0)
    {
		LOGE("Stop timeout");
    }
    
    mState = WK_RTSP_CTRL_STATE_STOPPED;
	mIsStopping = false;
	mRequest = NULL;
	// mStreamType = SK_STREAM_MODE_NULL;

    mRtsp->stop();
    mContext->sessionStop();
    (mLooper->getMessageQueue())->removeMessages(WK_RTSP_CTRL_GETPARAMETER);
	(mLooper->getMessageQueue())->removeMessages(WK_RTSP_CTRL_TIMEOUT);
	(mLooper->getMessageQueue())->removeMessages(WK_RTSP_CTRL_ERROR);
	LOGE("WKRtspCtrl stop finished");

	return true;
}
bool WKRtspCtrl::doStop()
{
	String cmdbuf;
	// cmdbuf.appendFormat("TEARDOWN %s RTSP/1.0\r\n",mContext->mUrl.string());
//	cmdbuf.appendFormat("TEARDOWN %s&hms_devid=4&prioritypopid=2 RTSP/1.0\r\n", mRtsp->mUrl);
	cmdbuf.appendFormat("TEARDOWN %s RTSP/1.0\r\n", mRtsp->mUrl);
	cmdbuf.appendFormat("CSeq: %u\r\n", mCseq);
	cmdbuf.appendFormat("Session: %s\r\n", mSession.string());
	cmdbuf.append("User-Agent: "WK_RTSP_UA"\r\n");
	cmdbuf.append("\r\n");    
	if (!mRtsp->sendCommand(cmdbuf.string()))
	{
		// return false;
		LOGE("send stop command error");
	}

	appendRequest(new WKRtspRequest(mCseq++, STOP));
	sendMessage(obainMessage(WK_RTSP_CTRL_TIMEOUT, mCseq - 1, 0), uptimeMillis() + 3000);
	return true;
}

bool WKRtspCtrl::pause()
{
    if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) != WK_RTSP_CTRL_STATE_CONNECTED)
    {
        return false;
    }
    AutoMutex _l(mLock);
    WKMessage* msg = obainMessage(WK_RTSP_CTRL_PAUSE);
	sendMessage(msg);

	//To avoid Dead Lock , timeout is 2 seconds
    if(mSignal.waitRelative(mLock, milliseconds(2000)) != 0)
		LOGE("pause timeout");
    return true;
}

bool WKRtspCtrl::doPause()
{
	String cmdbuf;
	cmdbuf.appendFormat("PAUSE %s RTSP/1.0\r\n", mContext->mUrl.string());
	cmdbuf.appendFormat("CSeq: %u\r\n", mCseq);
	cmdbuf.appendFormat("Session: %s\r\n", mSession.string());
	cmdbuf.append("User-Agent: "WK_RTSP_UA"\r\n");
	cmdbuf.append("\r\n");

	if (!mRtsp->sendCommand(cmdbuf.string()))
	{
		return false;
	}
	appendRequest(new WKRtspRequest(mCseq++, PAUSE));
    sendMessage(obainMessage(WK_RTSP_CTRL_TIMEOUT, mCseq - 1, 0), uptimeMillis() + 3000);
	LOGD("send pause seq %d", mCseq);
	return true;
}

bool WKRtspCtrl::getParameter(int burstMode)
{
    if ((mState & WK_RTSP_CTRL_STATE_CONNECTED) != WK_RTSP_CTRL_STATE_CONNECTED)
    {
        return false;
    }
	String cmdbuf;
	cmdbuf.appendFormat("GET_PARAMETER %s RTSP/1.0\r\n", mContext->mUrl.string());
	cmdbuf.appendFormat("CSeq: %u\r\n", mCseq);
	cmdbuf.appendFormat("Session: %s\r\n", mSession.string());
    cmdbuf.append("User-Agent: "WK_RTSP_UA"\r\n");

	cmdbuf.append("\r\n");

	if (!mRtsp->sendCommand(cmdbuf.string()))
	{
		LOGE("send getparameter errer");
		return false;
	}

    {
		appendRequest(new WKRtspRequest(mCseq++, GETPARAMETER));
    }
	return true;
}

void WKRtspCtrl::setState(WK_RTSP_CTRL_STATE state)
{
	AutoMutex _l(mLock);
	mState = state;
}

void WKRtspCtrl::appendRequest(WKRtspRequest* r)
{
	AutoMutex _l(mLock);
	WKRtspRequest* p = mRequest;
	if (p == NULL)
	{
		mRequest = r;
	}
	else
	{
		while (p->next != NULL)
		{
			p = p->next;
		}
		p->next = r;
	}
}

WKRtspRequest* WKRtspCtrl::findRequest(int seq)
{
	AutoMutex _l(mLock);
	WKRtspRequest* p = mRequest;
	WKRtspRequest* q = NULL;

	while (p != NULL)
	{
		if (p->mCseq == seq)
		{
			// 找到了
			if (q == NULL)
			{
				// 第一个就是，咋整
				mRequest = p->next;
			}
			else
			{
				q->next = p->next;
			}
			p->next = NULL;
			break;
		}
		q = p;
		p = p->next;
	}
	return p;
}
