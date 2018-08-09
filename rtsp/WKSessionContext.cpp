#include "WKSessionContext.h"
#include "WKTSPackage.h"
#include "WKTSPackageQueue.h"
#include "WKHandler.h"
#include "WKRtspResponseDecoder.h"
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "wk_android_std.h"
#include <String.h>
#include <stdlib.h>
#include <string.h>


#define MODULE_NAME "SKIPTV_SESSION_CONTEXT"
using namespace android;
WKSessionContext::WKSessionContext()
{
	mServerSocket = -1;
	//mQueue = new SKTSPackageQueue();
	mHoldSelf = this;
	mTsHandler = NULL;
	mRtspHandler = NULL;
	mEventHandler = NULL;
	mRtspThreadState = WK_THREAD_STATE_IDLE;
	mUDPThreadState = WK_THREAD_STATE_IDLE;
	mStreamType = WK_STREAM_IDLE;
	mMultifailCount = 0;
    mMul2UniIsFail = false;
	mHasMul2Uni = false;
	mServerSinfamily = AF_INET;	
}

void WKSessionContext::setServerSinfamily(short int sin_family)
{
	if(AF_INET != sin_family && AF_INET6 != sin_family)
	{
		LOGI("[%s:%d] Addr sin_family value invalid !! sin_family:%d.", __FUNCTION__, __LINE__, sin_family);
	}
	else
	{
		mServerSinfamily = sin_family;
	}
}

void WKSessionContext::sessionStop()
{
	AutoMutex _l(mLock);
	if (mServerSocket > 0)
		shutdown(mServerSocket,SHUT_RDWR);
	mServerSocket = -1;
	mRtspThreadState = WK_THREAD_STATE_IDLE;
	mUDPThreadState = WK_THREAD_STATE_IDLE;
}

WKSessionContext::~WKSessionContext()
{
	//mQueue = NULL;
	mTsHandler = NULL;
	mRtspHandler = NULL;
	mEventHandler = NULL;
}

#if 0
sp<SKTSPackage> WKSessionContext::obainTsPkg()
{
	// TODO:
	// context应持有处理TS流数据的handler，这里分配TS包的时候自动加上handler
	// return SKTSPackage::obain(tsHanler);
	return SKTSPackage::obain();
}

void WKSessionContext::enqueueTsPkg(sp<SKTSPackage> pkg)
{
	mQueue->enqueuePackage(pkg); 
}
#endif

void WKSessionContext::setTsHandler(WKHandler* h)
{
	mTsHandler = h;
}

void WKSessionContext::setRtspHandler(WKHandler* h)
{
	mRtspHandler = h;
}

void WKSessionContext::setEventHandler(WKHandler* h)
{
	mEventHandler = h;
}

// void WKSessionContext::setIGMPUrl(const char* url)
// {
//     mIgmpUrl.setTo(url);
// }
// const char* WKSessionContext::getIGMPUrl()
// {
//     return mIgmpUrl.string();
// }
THREAD_STATES WKSessionContext::getUDPThreadState()
{
	AutoMutex _l(mLock);
	return mUDPThreadState;
}
void WKSessionContext::setUDPThreadState(THREAD_STATES state)
{
	AutoMutex _l(mLock);
	mUDPThreadState = state;
}

THREAD_STATES WKSessionContext::getReceiveThreadState()
{
	// TODO:暂时先只处理UDP线程，此方法应同时处理RTSP和UDP线程
	// return mUDPThreadState;
	// 在组播状态下只有UDP线程的状态，在TCP承载方式下，只有RTSP线程状态，其他模式下UDP线程和RTSP线程状态一致
	AutoMutex _l(mLock);
	if ((mStreamType & WK_STREAM_MULTICAST) > 0)
		return mUDPThreadState;
	// if(mStreamType == SK_STREAM_LIVE_UNMULTICAST_TCP || mStreamType == SK_STREAM_SHIFT_UNMULTICAST_TCP)
	// 	return mRtspThreadState;
	return mRtspThreadState;
}


//TCP  stream : set RTSP receive state
//UDP stream : set UDP receive state
void WKSessionContext::setReceiveThreadState(THREAD_STATES state)
{
	AutoMutex _l(mLock);
	if ((mStreamType & WK_STREAM_MULTICAST) > 0)
	{
		mUDPThreadState = state;
		mRtspThreadState = WK_THREAD_STATE_IDLE;
	}
	else if (((mStreamType & WK_STREAM_MULTICAST) == 0) && ((mStreamType & WK_STREAM_UDP) == 0))
	{
		mUDPThreadState = WK_THREAD_STATE_IDLE;
		mRtspThreadState = state;
	}
	else
	{
		if((mStreamType & WK_STREAM_UDP) == 0)
		{
			mUDPThreadState = state;
			mRtspThreadState = state;
		}else
			{
				mUDPThreadState = state;
			}
	}
}

unsigned int WKSessionContext::getStreamType()
{
	return mStreamType;
}

void WKSessionContext::setStreamType(unsigned int t)
{
	mStreamType = t;
}
unsigned long WKSessionContext::getLocalIP()
{
    return getLocalIP(NULL, 0);
}

unsigned long WKSessionContext::getLocalIP(char* ifname, int size)
{
	struct ifaddrs * ifAddrStruct = NULL;
	void * tmpAddrPtr = NULL;
    unsigned long ret = 0;

	struct ifaddrs *ppp0addr = NULL, *eth0addr = NULL, *wlan0addr = NULL;
	getifaddrs(&ifAddrStruct);

	while (ifAddrStruct != NULL)
	{
        if(ifAddrStruct->ifa_addr == NULL || ifAddrStruct->ifa_name == NULL)
        {
            ifAddrStruct = ifAddrStruct->ifa_next;
            continue;
        }
        
		if (ifAddrStruct->ifa_addr->sa_family == AF_INET)
		{
			// check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			LOGI("%s IP Address [%s]", ifAddrStruct->ifa_name, addressBuffer);

            if (strncasecmp(ifAddrStruct->ifa_name, "lo", strlen("lo")) == 0)
			{
                ifAddrStruct = ifAddrStruct->ifa_next;
				continue;
			}

            //ret = ((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr.s_addr;

            /*if(ifname != NULL && size > 0)
            {
                strlcpy(ifname, ifAddrStruct->ifa_name, size);
            }*/
            
			if (strncasecmp(ifAddrStruct->ifa_name, "ppp0", strlen("ppp0")) == 0)
			{
				LOGI("find ppp0 ip is %s", addressBuffer);
				// return ((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr.s_addr;
				ppp0addr = ifAddrStruct;
				//return ret;
			}
			else if (strncasecmp(ifAddrStruct->ifa_name, "eth0", strlen("eth0")) == 0)
			{
				LOGI("find eth0 ip is %s", addressBuffer);
				// return ((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr.s_addr;
				eth0addr = ifAddrStruct;

				//return ret;
			}
			else if (strncasecmp(ifAddrStruct->ifa_name, "wlan0", strlen("wlan0")) == 0)
			{
				LOGI("find wlan0 ip is %s", addressBuffer);
				// return ((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr.s_addr;
				wlan0addr = ifAddrStruct;
                
				//return ret;
			}
			else
			{
				LOGE("find local ip not valid, continue...\n");
			}
		}
		else if (ifAddrStruct->ifa_addr->sa_family == AF_INET6)
		{
			// check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			LOGI("%s IP Address %s", ifAddrStruct->ifa_name, addressBuffer);
		}
		ifAddrStruct = ifAddrStruct->ifa_next;
	}
	
	if(NULL != ppp0addr)
	{
		ifAddrStruct = ppp0addr;
	}
	else if(NULL != eth0addr)
	{
		ifAddrStruct = eth0addr;
	}
	else if(NULL != wlan0addr)
	{
		ifAddrStruct = wlan0addr;
	}
	
	if(ifAddrStruct == NULL)
	{
		LOGI("[%s:%d] ifAddrStruct is NULL.", __FUNCTION__, __LINE__);
		return ret;
	}
	
	if(NULL != ifname && size > 0)
    {
        strncpy(ifname, ifAddrStruct->ifa_name, size);
    }

    ret = ((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr.s_addr;

	return ret;
}

void WKSessionContext::setHasRtp(bool has)
{
    mHasRtp = has;
}

bool WKSessionContext::getHasRtp()
{
    return mHasRtp;
}

