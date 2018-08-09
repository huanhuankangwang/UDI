#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <ctype.h>
#include "WKUDP.h"
#include "wk_android_std.h"
#include "WKMessage.h"
#include "WKSessionContext.h"
// #include "SKCyclerBuffer.h"
#include "WKTSPackage.h"
#include "WKMediaProcessor.h"


#include <string.h>


#define MODULE_NAME "WKIPTV_UDP"
#define NEW_RMEM_MAX                    (5 * 1024 * 1024)
using namespace android;
bool WKUDP::rmem_max_modified = false;

WKUDP::WKUDP()
{
	mSocket = -1;
	mIsMulticastIP = false;
	mIP = 0;
	mLocalIPString.setTo("0.0.0.0");
	mPort = 0;
	mThreadId = 0;
	mContext = NULL;
	mNextRTPSeq = -1;
    mIsThreadRunning = false;
	// mCyclerBuffer = new SKCyclerBuffer();
	mLooper = new WKLooper();
	if (mLooper->loop())
	{
		setLooper(mLooper);
		LOGI("WKUDP create looper success!");
	}
	else
		LOGE("SKRtsp create looper falid!");
}

WKUDP::~WKUDP()
{
	LOGE("WKUDP destroy");
    mLooper = NULL;
}

const char* WKUDP::getLocalIPString()
{
	//if (mLocalIPString == "0.0.0.0")
	//{
		char addressBuffer[INET_ADDRSTRLEN];
		unsigned long tmpAddr = mContext->getLocalIP();
		void * tmpAddrPtr = &tmpAddr;
		inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
		LOGI("Local IP Address %s", addressBuffer);
        if(mLocalIPString != addressBuffer)
		{
            LOGI("Set Local IP Address %s to mLocalIPString.", addressBuffer);
		    mLocalIPString.setTo(addressBuffer);
        }
	//}
	return mLocalIPString.string();
}


bool WKUDP::uniPlay(WKSessionContext* context, unsigned long ip, unsigned short port)
{
    AutoMutex _l(mLock);
    bool ret = false;
    if(create(context, ip, port))
    {
        sendMessage(obainMessage(WK_UDP_RECEIVE_START));
        ret = true;
    }

    return ret;
}


bool WKUDP::create(WKSessionContext* context, unsigned long ip, unsigned short port)
{
	if(context == NULL)
	{
		LOGE("Session conext is NULL");
		return false;
	}
	mContext = context;
	int fd;
	struct ifreq ifr;
	int new_rmem_max = NEW_RMEM_MAX;
	int reuse = 1;
	char localIfName[16] = {0};

	mIP = ip;
	mPort  = port;
	
	struct sockaddr_in local;
	/* 绑定本地地址和端口 */
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	LOGD("Create udp socket Begin,ip:%s port:%d", inet_ntoa(*((struct in_addr *)(&ip))), port);	
	
	/* 创建socket  */
	fd = socket(local.sin_family, SOCK_DGRAM, 0);
	if (fd <= 0)
	{
		LOGE("Create udp socket failed!");
		return false;
	}

    unsigned long localIP = mContext->getLocalIP(localIfName, sizeof(localIfName));

	if (IS_MULTICAST_IP(ip))
	{
		local.sin_addr.s_addr = ip;
		mIsMulticastIP = true;
	}
	else
	{
		const char *now_ip = NULL;

		local.sin_addr.s_addr = localIP;

		mIsMulticastIP = false;
	}
	// local.sin_port = port;
	local.sin_port = htons(port);
	if (rmem_max_modified == false)
	{

		/* dma: for Linux kernel tuning.   Linux kernel set socket receive buffer to 64K by default.
		   It works with low bit rate stream up to ~14Mbps. With higher bit rate (19 or above)Mbps,
		   Linux starts dropping UDP packets.   This has been verified by the following
		   /proc/net/snmp and /proc/net/sudp

		   We need to increase our UDP socket receive buffer size by using setsockopt().  Therefore,
		   /proc/sys/net/core/rmem_max needs to be changed.

		   NOTE: the following will set rmem_max to 200K if it was 64K.you can use Linux
		   shell command "echo 200000 > /proc/sys/net/core/rmem_max" to achieve the same
		   effect.
		*/
		int f;
		char buf[80];
		int rmax;

		f = open("/proc/sys/net/core/rmem_max", O_RDONLY);
		if (f != -1)
		{

			read(f, buf, sizeof(buf));
			sscanf(buf, "%d", &rmax);
			LOGE("=======================net rmax %d===========================",rmax);
			close(f);
			if (rmax < NEW_RMEM_MAX)
			{

				/* it is the default value, make it bigger */
				sprintf(buf, "%d", NEW_RMEM_MAX);
				f = open("/proc/sys/net/core/rmem_max", O_WRONLY);
				write(f, buf, strlen(buf) + 1);
				close(f);
				new_rmem_max = NEW_RMEM_MAX;
			}
			else
			{
				new_rmem_max = rmax;
			}
			f = open("/proc/sys/net/core/rmem_max", O_RDONLY);
			read(f, buf, sizeof(buf));
			sscanf(buf, "%d", &rmax);
			LOGE("=======================net rmax %d===========================",rmax);
			close(f);
		}

		rmem_max_modified = true;
	}

	int	opt_flag = 1;
	int	opt_len = 0 ;
	

	opt_flag = 5 * 1024 * 1024;


	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&opt_flag, sizeof(opt_flag)) == -1)
	{
		LOGE("cannot SO_RCVBUF  rcv buffer size: %d\n", opt_flag);
	}

	opt_len = sizeof(opt_flag);
	if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&opt_flag, (socklen_t*)&opt_len) == -1)
	{
		LOGE("getsockopt() failed to get UDP rcv buffer size: %d\n", opt_flag);
	}
	else
	{
		LOGD( "======UDP SO_RCVBUF  size: %d, %d\n", opt_flag, opt_len);
	}	

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) != 0)
	{

		perror("Set socket opt Failed");
		close(fd);
		return false;
	}

	if (bind(fd, (struct sockaddr *)&local, sizeof(local)) == -1)
	{

		perror("bind socket Failed");
		close(fd);
		return false;
	}

	LOGI("Create socket OK! fd:%d", fd);

	mSocket = fd;
#ifdef SK_SUPPORT_NAT
    mContext->setmudpsd(mSocket);
#endif

	return true;
}
bool WKUDP::send(const char* data)
{
	return true;
}

void WKUDP::stoping()
{
	if(mContext != NULL)
		mContext->setUDPThreadState(WK_THREAD_STATE_EXITING);
}

//stop  and exit recv thread.
void WKUDP::stop()
{
    AutoMutex _l(mLock);
	LOGD("stop udp thread");

	//select timeout is 1seconds , to force recv exit , force close socket. 
	if(mSocket > 0)
		close(mSocket);
	mSocket = -1;
	
	if(!mIsThreadRunning)
		return;
	
	if(mContext!= NULL)
	{
		mContext->setUDPThreadState(WK_THREAD_STATE_EXITING);
		while(mIsThreadRunning)
		{
            mContext->setUDPThreadState(WK_THREAD_STATE_EXITING);
			usleep(100);
		}
	}

}
bool WKUDP::joinMulticast(int *errornumb, bool fromFcc)
{
    AutoMutex _l(mLock);
	if (!mIsMulticastIP)
	{
		LOGE("udp not multicast ip!");
		return false;
	}
	/* 加入多播组(IGMPV1) */
    //sp<SKHandler> h = mContext->mEventHandler;
    //h->sendMessage(h->obainMessage(EVENT_MEDIA_CLEARBUF));

	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = mIP;

	// TODO:
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);//mContext->getLocalIP();

	LOGD("udp_join_multicast mSocket:%d ip:%lu local ip:%lu, ip str:%s", mSocket, mIP, mContext->getLocalIP(), inet_ntoa(*((struct in_addr *)(&mIP))));




	if (setsockopt(mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) == 0)
	{
		LOGI("Join the multicast ok!");
		// mContext ->setReceiveThreadState();
		WKMessage* msg = obainMessage(WK_UDP_RECEIVE_START);
		sendMessage(msg);
		return true;
	}
	else
	{
		LOGE("Join the multicast failed!!!%s,%d", strerror(errno), errno);
		if (errno == 98)		// 已经在组播中，返回true
			return true;
		else
		{
			*errornumb = errno;
		}
		return false;
	}
}


//Leave Multicast and exit recv thread.
bool WKUDP::leaveMulticast()
{
    AutoMutex _l(mLock);
	LOGE("leaveMulticast ");
	if (!mIsMulticastIP)
	{
		LOGE("udp not multicast ip!");
		return false;
	}
    
	if(mContext == NULL)
	{
		return false;
	}
    
	mContext->setUDPThreadState(WK_THREAD_STATE_EXITING);
	LOGE("leaveMulticast  111 mIsThreadRunning=%d\n", mIsThreadRunning);
    while(mIsThreadRunning)
	{
        usleep(100);
    }
	LOGE("leaveMulticast  222 mIsThreadRunning=%d\n", mIsThreadRunning);

	/* 退出组播 */

	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = mIP;

    mreq.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt(mSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) == 0)
	
	{
		LOGI("Leave the multicast ok!");
	}
	else
	{
		LOGE("leave the multicast error");
	}
	close(mSocket);
	mSocket = -1;
//    sp<SKHandler> h = mContext->mEventHandler;
//    h->sendMessage(h->obainMessage(EVENT_MEDIA_CLEARBUF));

    LOGD("udp recv thread is exit!");
	return true;
}

void WKUDP::dispatchMessage(WKMessage* msg)
{
	switch (msg->what)
	{
	case WK_UDP_RECEIVE_START:
	{
		LOGD("udp recive thread start msg!!!");
		mContext->setUDPThreadState(WK_THREAD_STATE_RUNNING);
		looper_thread();
		break;
	}
	default:
		LOGI("do nothing");
		break;
	}
}
int WKUDP::recvData(char *buf, int size, int timeout, struct sockaddr *from , int *fromlen)
{
	if (mSocket < 0)
	{
		LOGE("[%s]%d ===============\n", __FUNCTION__ , __LINE__);
		// TODO
		// 错误上层如何处理？
		// sp<SKHandler> h = mContext->mRtspHandler;
		// h->sendMessage(h->obainMessage(SK_RTSP_SOCKET_ERROR));
		// mContext->mRtspThreadState = SK_THREAD_STATE_EXITING;
    		if((mContext->getStreamType() & (WK_STREAM_UDP)))
		{
		    mContext->setUDPThreadState(WK_THREAD_STATE_EXITING);
		}
		return -1;
	}
	fd_set rfds, efds;
	int ret;
	struct timeval  timeVal;

	/* 检查是否有数据可读 */
	FD_ZERO(&rfds);
	FD_SET(mSocket, &rfds);
	FD_ZERO(&efds);
	FD_SET(mSocket, &efds);

	timeVal.tv_sec = timeout / 1000;
	timeVal.tv_usec = (timeout % 1000) * 1000;

	if (timeout < 0)
	{
		ret = select(mSocket + 1, &rfds, NULL, &efds, NULL);
	}
	else
	{
		ret = select(mSocket + 1, &rfds, NULL, &efds, &timeVal);
	}
	// LOGI("select return %d",ret);
	if (0 < ret && FD_ISSET(mSocket, &rfds))
	{
		int recvsize = -1;
		// recvsize = recv(mContext->mServerSocket, buf, size, 0);
		recvsize = recvfrom(mSocket, buf, size, 0, from, (socklen_t*)&fromlen);
		if (recvsize >= 0)
		{
			return recvsize;
		}
	}
	else if (0 == ret)
	{
		// 链接尚未断开，无数据可读
		return 0;
	}
	// if(mContext->mRtspThreadState == SK_THREAD_STATE_RUNNING) // running的时候出现这种情况属异常，其他情况可能是正常退出
	// {
	// 	sp<SKHandler> h = mContext->mRtspHandler;
	// 	h->sendMessage(h->obainMessage(SK_RTSP_SOCKET_ERROR));
	// 	mContext->mRtspThreadState = SK_THREAD_STATE_EXITING;
	// }
	return -1;
}

int rtp_buffer_compare_seqnum (unsigned short seqnum1, unsigned short seqnum2)
{
  return (signed short) (seqnum2 - seqnum1);
}

char* WKUDP::get_rtp_or_ts_buffer(char* buf, int size, int& media_size, int& read_size, RtpPackageHeader **in_rtpHeader)
{
	int skip_size = 0;
    struct sockaddr_in from;

	int slen = sizeof(from);
    
    read_size = recvData(buf, size, 300/*1 * 1000*/, (struct sockaddr *) &from, &slen);
	if (read_size <= 0)
	{
		// 无数据可读
		return NULL;
	}
    
	if ((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0 
        && (mContext->mMultifailCount < MAX_MULTIFAILED))
	{
		mContext->mMultifailCount = 0;
	}

    //	LOGE("receive udp length %u \n",next_read_size ); 
	if (buf[0] != 0x47)	// is ts package? or rtp package
	{
		RtpPackageHeader *rtpHeader = (RtpPackageHeader*)buf;
		unsigned int extn_len = 0;
        
		if (rtpHeader->ph_v == 2 && (12 + (4 * rtpHeader->ph_cc) <= read_size))
		{
            *in_rtpHeader = rtpHeader;
			skip_size = (12 + (rtpHeader->ph_cc * 4));
			if (rtpHeader->ph_x)
			{
				extn_len = (buf[2+skip_size] << 8) | buf[3+skip_size];
				skip_size += (extn_len + 1) * 4;
			}
			/*if the same multicast packet like the last , discard it */
			// if(ret == 0 && IS_MULTICAST_IP(ip))
			// {
			//     skip_size = size;
			// }
		}
		else
		{
			if(strncmp(buf , "ZXV" , 3) == 0)
			{
				//LOGD("receive Server NAT packet \n");
			}
			else
			{
				LOGD(" (%s) ph_v=%d rtpHeader->ph_cc=%d size=%d buf[0]:%x \n", __FUNCTION__, rtpHeader->ph_v, rtpHeader->ph_cc, read_size, buf[0]);
			}

            return NULL;
		}
	}
    
	if (skip_size >= read_size || skip_size < 0)
	{
		LOGE(" (%s) Invalid skip_size %d\n", __FUNCTION__, skip_size);
		skip_size = 0;
		return NULL;
	}
    
	// TODO: 排除fec包
	if (buf[skip_size] != 0x47)
	{
		LOGE("bad ts package");
		return NULL;
	}

    media_size = read_size - skip_size;
    
    return (buf + skip_size);
}

void WKUDP::looper_thread()
{
	//char buf[65536];

    //for rtp sort, here define double buffer
	char buf[MTU_MAX];  //UDP package max size
	char prebuf[MTU_MAX];  //UDP package max size

    char* buffer = NULL;
    
    unsigned short preseq = 0;
	int pre_size = -1;
    
	int next_read_size;
	int cur_read_size;
	int skip_size;
    int media_size = 0;
    int read_size = 0;
    unsigned int seq = 0xAAAAAAAA;

	struct sockaddr_in from;

	int slen = sizeof(from);

    int64_t now = uptimeMillis();
	int64_t last_recv_time = now;
	
#if defined(SK_SUPPORT_HI_CU)
	int recv_timeout = 3000;
#else
	int recv_timeout = 2000;
#endif

	mNextRTPSeq = 0;//0xFFFF;
	bool has_recvd_rtp = false;
    bool has_recvd_data = false;
	
   	WKMediaProcessor* processor = sk_mc_get_processor();
	LOGD("looper_thread begin \n");
   	mIsThreadRunning = true;

	LOGE("UDP recv thread Start .....%d \n", gettid());
	nice(-40);
	while ((mContext->getUDPThreadState() == WK_THREAD_STATE_RUNNING) && (mContext->getStreamType() & (WK_STREAM_UDP)))
	{
        now = uptimeMillis();
        if(now - last_recv_time > recv_timeout)
		{
            if(((mContext->getStreamType() & WK_STREAM_MULTICAST) > 0)
			)
            {
				if (!mContext->mMul2UniIsFail && (sk_mc_get_need_mul2uni() == 1))
				{
					mContext->mMultifailCount++;

					sk_mc_set_mul2uni(1);
    				WKHandler* h = mContext->mEventHandler;
    				h->sendMessage(h->obainMessage(EVENT_MEDIA_MUL2UNI));
    				LOGE("muti time out !!!");
    				break;
			    }
            }
            else
            {
                if(mRtspCtrl != NULL && (mRtspCtrl->getState() != WK_RTSP_CTRL_STATE_PAUSEING))
                {
            	}
            }
		}
        
    	if (mSocket < 0)
    	{
		    LOGE("[%s]%d ======mSocket < 0 break=========\n", __FUNCTION__ , __LINE__);
            break;
        }
       //RTP_SEQ_SORT
        RtpPackageHeader *rtpHeader = NULL;
        media_size = 0;
        read_size = 0;

		char* buffer = get_rtp_or_ts_buffer(buf, sizeof(buf), media_size, read_size, &rtpHeader);

        if(buffer == NULL)
        {
            continue;
        }
        
        if(rtpHeader != NULL)
        {
            unsigned short usseq = ntohs(rtpHeader->ph_seq);
            unsigned int timestamp = ntohl(rtpHeader->ph_ts);
            seq = usseq;
    		if(!has_recvd_rtp)
            {   
            	LOGD("received first rtp packet, seq:%u", usseq);
                has_recvd_rtp = true;
			}
		}

        {
            processor->InputDataMediaProcessor(buffer, media_size, seq);
        }

		last_recv_time = uptimeMillis();
		has_recvd_data = true;
        
	}
    
	if((mContext->getStreamType() & (WK_STREAM_UDP)))
	{
    	mContext->setUDPThreadState(WK_THREAD_STATE_EXITED);
	}
	LOGE("udp recive thread exit!!!");
	// 通知rtsp control线程收流线程已退出
	// sp<SKHandler> h = mContext->mRtspHandler;
	// h->sendMessage(h->obainMessage(SK_RTSP_RECEIVE_STOP));
    mIsThreadRunning = false;
	return ;
}


void WKUDP::setRtspCtrl(WKRtspCtrl* rc)
{
    mRtspCtrl = rc;
}
