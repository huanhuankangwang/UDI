#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <ctype.h>
#include "WKURLParser.h"
#include "wk_android_std.h"
#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define MODULE_NAME "SK_URL_PARSER"
using namespace android;

WKURLParser::WKURLParser()
{
	reset();
}

WKURLParser::~WKURLParser()
{
	//:do nothing
}
void WKURLParser::reset()
{
	mLiveMultiUrl.clear();
	mShiftUrl.clear();
	mLiveRtspUrl.clear();
	mIsChannel = false;
	mTrickable = false;
	mIsBookmark = false;

	mMulticastIP = 0;
	mMulticastPort = 0;

    mTimeShiftLength = 0;
}
const char* WKURLParser::getBookmark()
{
	if (mIsBookmark)
		return bookmark.string();
	return NULL;
}
bool WKURLParser::parse(const char* url)
{
    String strTemp(url);
	const char *temp = strTemp.string();
	LOGI("setDataSource:%s", url);
	reset();
	char *p = NULL;
	char *q = NULL;

	p = strstr(temp, "?sky:");
	if (p != 0)
	{
		*p = '\0';
		p += strlen("?sky:");
		LOGI("skyworth url protocl:%s", p);

		if ((q = strstr(p, "timeshift=")) != NULL)
		{
			if (atoi(q + strlen("timeshift=")) > 0)
			{
				mTrickable = true;
				LOGI("data source trick enable");
			}
			else
			{
				mTrickable = false;
				LOGI("data source trick unable");
			}
		}
		if ((q = strstr(p, "&channel=1")) != NULL)
		{
			mIsChannel = true;
			//}else(...)
			LOGI("data source is channel");
		}
		else
		{
			mIsChannel = false;
			LOGI("data source is vod");
		}
		if ((q = strstr(p, "&bookmark=")) != NULL)
		{
			mIsBookmark = true;
			char *r  = NULL;
			q += strlen("&bookmark=");
			if ((r = strchr(q, '&')) == NULL)
			{
				bookmark.setTo(q);
			}
			else
			{
				bookmark.clear();
				bookmark.append(q, r - q);
			}
			LOGE("bookmark:%s", bookmark.string());
		}
        
        if ((q = strstr(p, "timeshiftlength=")) != NULL)
		{
            mTimeShiftLength = atoi(q + strlen("timeshiftlength="));
			LOGI("timeshiftlenfth:%d", mTimeShiftLength);
		}
	}							  // :for taizhou
	else
	{
		mIsChannel = false;
	}
	// :end

	if (strncasecmp(temp, "igmp://", strlen("igmp://")) == 0)//:multicast
	{
		p = strchr(temp, '|');
		char *p2 = strchr(temp, ';');
		
		if (p != NULL && (p < p2 || p2 == NULL))//for zjtz
		{
			// live url have igmp and rtsp
			*p++ = '\0';
			mLiveMultiUrl.setTo((char*)temp);
			if ((q = strchr(p, ';')) != NULL)//:have shift url
			{
				*q++ = '\0';
				mLiveRtspUrl.setTo((char*)p);
				//:TODO
				//:if mTrickable = true;
				mShiftUrl.setTo((char*)q);
				LOGI("live igmp:%s", mLiveMultiUrl.string());
				LOGI("live rtsp:%s", mLiveRtspUrl.string());
				LOGI("shift:%s", mShiftUrl.string());
			}
			else
			{
				//:olny multicast url
				mLiveRtspUrl.setTo((char*)p);
				if(mIsChannel && mTrickable)
                {
                    mShiftUrl.setTo((char*)p);
                }
                else
                {
                    mTrickable = false;
    				LOGI("only live multicast %s,rtsp %s", mLiveMultiUrl.string(), mLiveRtspUrl.string());
    				mShiftUrl.clear();
                }
			}
		}
		else if (p2 != NULL && (p2 < p || p == NULL))//channel url igmp, timeshifturl rtsp
		{
			*p2++ = '\0';
			mLiveMultiUrl.setTo((char*)temp);
			mLiveRtspUrl.setTo((char*)p2);
			mShiftUrl.setTo((char*)p2);
			LOGI("[%s:%d]live igmp:%s", __FUNCTION__, __LINE__, mLiveMultiUrl.string());
			LOGI("[%s:%d]live rtsp:%s", __FUNCTION__, __LINE__, mLiveRtspUrl.string());
			LOGI("[%s:%d]shift:%s", __FUNCTION__, __LINE__, mShiftUrl.string());			
		}
		else// only igmp url
		{
			mLiveMultiUrl.setTo((char*)temp);
			mTrickable = false;
		}

		if (!parseIGMPUrl(mLiveMultiUrl.string()))
		{

			mMulticastIP = 0;
			mMulticastPort = 0;

		}

        mIsMultiLive = true;
        
        if ((sk_mc_get_mul2uni() == 1) && (sk_mc_get_need_mul2uni() == 1))
		{
            //有合法的转单播地址可以转单播
			if(mLiveRtspUrl.find("rtsp://") == 0 || mShiftUrl.find("rtsp://") == 0 || mLiveRtspUrl.find("http://") == 0 || mShiftUrl.find("http://") == 0)
            {
                mIsMultiLive = false;
            }
		}
	}
	else if (strncasecmp(temp, "rtsp://", strlen("rtsp://")) == 0 || strncasecmp(temp, "http://", 7) == 0 || strncasecmp(temp, "https://", 8) == 0)//:unicast
	{
		if ((p = strchr(temp, ';')) != NULL)//:have shift url
		{
			*p++ = '\0';
			mLiveRtspUrl.setTo((char*)temp);
			//:TODO
			//:if mTrickable = true;
			mShiftUrl.setTo((char*)p);
			LOGI("live rtsp:%s", mLiveRtspUrl.string());
			LOGI("shift:%s", mShiftUrl.string());
		}
		else
		{
			//:olny multicast url
			// igmp.setTo((char*)temp);
			mLiveRtspUrl.setTo((char*)temp);
			mTrickable = false;
			LOGI("only live rtsp:%s", mShiftUrl.string());
		}
		mIsMultiLive = false;
	}
	else
	{//:TODO http
		LOGE("bad url!");
		return false;
	}

#if 1 //defined(SK_SUPPORT_HI_CU) || defined(SK_SUPPORT_HB_CTC) 
    if(mIsChannel && mTrickable && mTimeShiftLength <= 0)
    {

 mTimeShiftLength = 3600; //海南联通没有下发TimeshiftLength, 如果频道支持时移, 默认可时移1小时.

        
    }
#endif
    
	return true;
}
bool WKURLParser::parseIGMPUrl(const char* url)
{
	if (strncasecmp(url, "igmp://", strlen("igmp://")) != 0)
	{
		LOGE("invaild igmp url:%s", url);
		return false;
	}
	const char *p, *q;
	char tmp[256] = {0};
	p = url + strlen("igmp://");



	q = strchr(p, ':');


	if (q == 0)
	{
		LOGE("invaild url,can not find igmp port!");
		return false;
	}
	strncpy(tmp, p, q - p);
	LOGD("igmp ip:%s", tmp);

	mMulticastIP = inet_addr(tmp);

	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, ++q);

	mMulticastPort = (unsigned short)strtol(tmp, NULL, 0);
	LOGD("igmp port:%d", mMulticastPort);

	// 在UDP对象中实现网络字节序转换
	// mMulticastPort = htons(mMulticastPort);
	return true;
}

const char* WKURLParser::getShiftUrl()
{
	return mShiftUrl.string();
}
const char* WKURLParser::getLiveMultiUrl()
{
	return mLiveMultiUrl.string();
}
const char* WKURLParser::getLiveRtspUrl()
{
    if(mLiveRtspUrl.isEmpty())
    {
        return mShiftUrl.string();
    }
    
	return mLiveRtspUrl.string();
}

unsigned long WKURLParser::getMulticastIP()
{
	return mMulticastIP;
}

unsigned short WKURLParser::getMulticastPort()
{
	return mMulticastPort;
}
bool WKURLParser::isMultiLive()
{
	return mIsMultiLive;
}
