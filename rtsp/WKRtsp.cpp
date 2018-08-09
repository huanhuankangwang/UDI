#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/times.h>
#include <net/if.h>

#include "WKRtsp.h"
#include "WKSessionContext.h"
#include "WKRtspResponseDecoder.h"
#include "WKHandler.h"
#include "WKMessage.h"
#include "WKLooper.h"
#include "WKTSPackage.h"
#include "WKMediaProcessor.h"
#include "WKNetUtils.h"
#include "unistd.h"
#include <sys/poll.h>

#include <stdlib.h>
#include <String.h>
#include <stdlib.h>
#include <string.h>

#include "wk_android_message.h"


#define MODULE_NAME "WKIPTV_RTSP"


using namespace android;
WKRtsp::WKRtsp()
{
	m_rrs_failed[MAX_RRS_FAILED] = {0, };
	m_rrs_skiped[MAX_RRS_FAILED] = {0, };

	mUrl = NULL;
	mServerName = NULL;
	mPort = 0;
	mRtspToken[8] = {0, };

	//backup服务器
	mBackupNum = 0;
	char *mBackupServers[MAX_BACKUP_SERVER] = {NULL, };
	mBackupPorts[MAX_BACKUP_SERVER] = {0, };


	// mServerSocket = 0;
	// mNextCseq = 0;
	mThreadId = 0;
	mContext = NULL;
	mLooper = new WKLooper();
    mIsThreadRunning = false;
	if (mLooper->loop())
	{
		setLooper(mLooper);
		LOGI("WKRtsp create looper success!");
	}
	else
	{
		LOGE("WKRtsp create looper falid!");
	}

}
WKRtsp::~WKRtsp()
{
	LOGE("WKRtsp destroy");
	mContext = NULL;
    
    if (mUrl != NULL)
		free(mUrl);
    
	if (mServerName != NULL)
		free(mServerName);
    
	if (mLooper != NULL)
	{
		sendMessage(WKMessage::obain(NULL));
	}
	//:TODO should wait for looper thread exit??
	mLooper = NULL;

}


/*解析url*/
bool WKRtsp::rtsp_dissect_url(const char *url)
{

	const char *uptr;
	const char *nextslash, *nextcolon, *rightbracket;
	int hostlen;

	if (mUrl != NULL || mServerName != NULL)
	{
		LOGD("client url or server_name is not null");
		if (mUrl != NULL)
			free(mUrl);
		if (mServerName != NULL)
			free(mServerName);
		// return false;
	}
	uptr = url;
	mUrl = strdup(url);
	if (strncasecmp("rtsp://", url, strlen("rtsp://")) == 0)
	{
		uptr += strlen("rtsp://");
		memset(mRtspToken, 0, sizeof(mRtspToken));
		strncpy(mRtspToken, "rtsp", sizeof(mRtspToken) - 1);
	}
	else if (strncasecmp("rtspu://", url, strlen("rtspu://")) == 0
			 || strncasecmp("rtspt://", url, strlen("rtspt://")) == 0)
	{

		memset(mRtspToken, 0, sizeof(mRtspToken));
		strncpy(mRtspToken, url, strlen("rtspt"));
		uptr += strlen("rtspu://");
		memset(mUrl, 0, strlen(url));
		strcpy(mUrl, "rtsp://");
		strncpy(mUrl + strlen("rtsp://"), uptr, strlen(url) - strlen("rtsp://"));
	}
	else
	{
		LOGE("rtsp token is wrong!");
		return false;
	}

	mPort = 554;
	rightbracket = NULL;
	if (*uptr == '[')
	{

		uptr++; // don't include the '['
		rightbracket = strchr(uptr, ']');
		if (rightbracket != NULL)
		{
			mContext->setServerSinfamily(AF_INET6);
			// literal IPv6 address
			if (rightbracket[1] == ':')
			{

				nextcolon = rightbracket + 1;
			}
			else
			{

				nextcolon = NULL;
			}
			nextslash = strchr(rightbracket, '/');
		}
		else
		{
			LOGE("url [] not match!");
			return false;
		}
	}
	else
	{

		nextslash = strchr(uptr, '/');
		nextcolon = strchr(uptr, ':');
	}

	if (nextslash != NULL || nextcolon != NULL)
	{

		if (nextcolon != NULL && (nextcolon < nextslash || nextslash == NULL))
		{

			hostlen = nextcolon - uptr;
			// have a port number
			nextcolon++;
			mPort = 0;
			while (isdigit(*nextcolon))
			{

				mPort *= 10;
				mPort += *nextcolon - '0';
				nextcolon++;
			}
			if (mPort == 0 || (*nextcolon != '/' && *nextcolon != '\0'))
			{
				LOGE("parse server port err!");
				return false;
			}
		}
		else
		{
			hostlen = nextslash - uptr;
		}
		if (rightbracket != NULL)
		{

			hostlen--;
		}
		if (hostlen == 0)
		{
			LOGE("parse host err");
			return false;
		}
		mServerName = (char *)malloc(hostlen + 1);
		if (mServerName == NULL)
		{
			LOGE("malloc space for server name err");
			return false;
		}
		memcpy(mServerName, uptr, hostlen);
		mServerName[hostlen] = '\0';
	}
	else
	{

		if (*uptr == '\0')
		{

			return false;
		}
		mServerName = strdup(uptr);
		if (mServerName == NULL)
		{

			return false;
		}
		if (rightbracket != NULL)
		{

			mServerName[strlen(mServerName) - 1] = '\0';
		}
	}

	//分析backup server
	mBackupNum  = 0;

	if (uptr = strstr(mUrl, "rrsiplist="))// for 四川电信
	{
		while(uptr != NULL && mBackupNum < MAX_BACKUP_SERVER)
		{
			nextcolon = uptr;
			while(*nextcolon != ',' && *nextcolon != ':' && *nextcolon != '\0' && *nextcolon != '&')
			{
				nextcolon++;
			}
			hostlen = nextcolon - uptr - strlen("rrsiplist=");
			if(hostlen > 0)
			{
			
				//取得备份rrs地址
				mBackupServers[mBackupNum] = (char*)malloc(hostlen + 1);
				strncpy(mBackupServers[mBackupNum], uptr + strlen("rrsiplist="), hostlen);
				mBackupServers[mBackupNum][hostlen] = '\0';
				if(*nextcolon == ':')
				{
					nextcolon++;
					mBackupPorts[mBackupNum] = 0;
					while (isdigit(*nextcolon))
					{

						mBackupPorts[mBackupNum] *= 10;
						mBackupPorts[mBackupNum] += *nextcolon - '0';
						nextcolon++;
					}
				}
				else
				{
					mBackupPorts[mBackupNum] = 554;
				}
				mBackupNum++;
			}
			uptr += strlen("rrsiplist=");
			uptr = strstr(uptr, "rrsiplist=");
		}
	}
	else
	{
		uptr = strstr(mUrl, "rrsip=");
		while (uptr != NULL && mBackupNum < MAX_BACKUP_SERVER)
		{

			nextcolon = uptr;
			while (*nextcolon != ',' && *nextcolon != ':' && *nextcolon != '\0' && *nextcolon != '&')
			{

				nextcolon++;
			}
			hostlen = nextcolon - uptr - strlen("rrsip=");
			if (hostlen > 0)
			{

				//取得备份rrs地址
				mBackupServers[mBackupNum] = (char*)malloc(hostlen + 1);
				strncpy(mBackupServers[mBackupNum], uptr + strlen("rrsip="), hostlen);
				mBackupServers[mBackupNum][hostlen] = '\0';

				//计算备份的rrs port
				if (*nextcolon == ':')
				{

					nextcolon++;
					mBackupPorts[mBackupNum] = 0;
					while (isdigit(*nextcolon))
					{

						mBackupPorts[mBackupNum] *= 10;
						mBackupPorts[mBackupNum] += *nextcolon - '0';
						nextcolon++;
					}
				}
				else
				{

					mBackupPorts[mBackupNum] = 554;
				}
				mBackupNum++;
			}
			uptr += strlen("rrsip=");
			uptr = strstr(uptr, "rrsip=");
		}
	}

	//删除URL中的rrsip
	uptr = strstr(mUrl, "rrsiplist");//for 四川电信
	if (uptr == NULL)
	{
		uptr = strstr(mUrl, "rrsip") ;
	}
	if (uptr)
	{

		//如果没有其他参数,则删除'?'
		if (*(uptr - 1) == '?' &&  *nextcolon == '\0')
		{

			uptr--;
		}
		if (*nextcolon != '\0')
		{

			nextcolon++;
		}
		memmove((char *)uptr, nextcolon, mUrl + strlen(mUrl) + 1 - nextcolon);
	}

	return true;
}

bool WKRtsp::rtsp_create_client(WKSessionContext* context, int *er)
{

	char *converted_url = NULL;
	int err;
	if(context == NULL)
		return false;
	
	mContext = context;
    
    LOGD("rtsp create client url %s", context->mUrl.data());
	converted_url = convert_url(context->mUrl.data());
    
	if(mContext->mServerSocket != -1)
	{
		LOGD("redirect , stop client first\n");
		stop();
	}
	mContext->mServerSocket = -1;

	/*解析url*/
	if (!rtsp_dissect_url(converted_url))
	{
		free(converted_url);
		LOGE("[RTSP] (%s) Couldn't decode url\n", __FUNCTION__);
		err = MEDIA_ERROR_INVALID_URL;
		return false;
	}
	free(converted_url);

	err = rtsp_create_socket();
	if (err != 0)
	{
		*er = errno;
		LOGE("[RTSP] (%s) Couldn't create socket %d\n", __FUNCTION__, errno);
		err = MEDIA_ERROR_SYSTEM_FATAL;
		return false;
	}
	WKMessage* msg = obainMessage(WK_RTSP_RECEIVE_START);
	sendMessage(msg);
	return true;
}

int WKRtsp::rtsp_create_socket()
{
	struct sockaddr_in sockaddr;
	in_addr_t serverip;

    struct ifreq ifr = {0};	
	struct timeval  timeVal = {0};
	int i, n;

	int optval = 64 * 1024;


	int unblock = 1;
	fd_set  rset, wset;
	int ret, err1, err2;
	int skt = -1;
	struct linger lingerOpt;
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;
	int passtime = 0;
    unsigned long localIP = 0;
    char localIfName[16] = {0};

	if (mContext->mServerSocket != -1)
	{
		LOGI("context server socket already create");
		return 0;
	}

	if (mServerName == NULL)
	{
		LOGE("no connect server name");
		return -1;
	}

	LOGI("[RTSP] (%s) connect %s:%d\n", __FUNCTION__, mServerName , mPort);
	if (!is_ipaddr(mContext->getServerSinfamily(), mServerName))
	{

		struct hostent *h = gethostbyname(mServerName);
		if (h != NULL)
		{
			memcpy(&serverip, h->h_addr_list[0], sizeof(serverip));
		}
		else
		{
			LOGE("can not get server ip from server name,is the DNS is crrect?");
			return -1;
		}
	}
	else
	{
		serverip = inet_addr(mServerName);
	}
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family =  mContext->getServerSinfamily();
	sockaddr.sin_port = htons(mPort);
	sockaddr.sin_addr = *(struct in_addr *)(&serverip);

	memset(m_rrs_skiped, 0, sizeof(m_rrs_skiped));
	LOGD("11111");

	//检查是否所有url里的ip都在失败表内, 如果是则重置失败表中所有本次url包括的ip
	if (rtsp_failedrrs_query(serverip, passtime) != NULL)
	{

		int flag = 0;
		for (i = 0; i < mBackupNum; i++)
		{

			if (rtsp_failedrrs_query(inet_addr(mBackupServers[i]), passtime) == NULL)
			{

				flag = 1;
				break;
			}
		}
		if (flag == 0)
		{

			LOGI("[RTSP](%s) all rrs ip are in FAIL_TABLE, clear it first!\n", __FUNCTION__);

			unsigned long *p_rrs = NULL;
			p_rrs = rtsp_failedrrs_query(serverip, passtime);
			if (p_rrs != NULL)
			{
				*p_rrs = 0;
			}
			else
			{
				LOGE("[RTSP](%s) ERROR, should not go here! a\n", __FUNCTION__);
			}
			for (i = 0; i < mBackupNum; i++)
			{

				p_rrs = rtsp_failedrrs_query(inet_addr(mBackupServers[i]), passtime);
				if (p_rrs != NULL)
				{
					*p_rrs = 0;
				}
				else
				{

					LOGE("[RTSP](%s) ERROR, should not go here! b\n", __FUNCTION__);
				}
			}
		}
		else
		{
			LOGI("[RTSP](%s) server ip is in FAIL_TABLE, skip!\n", __FUNCTION__);
		}
	}
	LOGD("222222222");
RE_CHECK:

	for (i = 0; i < mBackupNum + 2; i++)
	{

		//如果当前连接的ip在失败表内, 则跳到取下一个backup server
		if (rtsp_failedrrs_query(*((unsigned long *)&sockaddr.sin_addr), passtime) != NULL)
		{
			LOGD("[RTSP](%s) current ip is in FAIL_TABLE, skip!\n", __FUNCTION__);
			goto NEXT_RRS;
		}

		skt = socket(sockaddr.sin_family, SOCK_STREAM, 0);		
		if (skt == -1)
		{
			LOGE("create socket fails");
			continue;
		}


		optval =  65536;

		setsockopt(skt, SOL_SOCKET, SO_RCVBUF, (void *)&optval, sizeof(optval));
		setsockopt(skt, SOL_SOCKET, SO_LINGER, (void *) &lingerOpt, sizeof(lingerOpt));
		optval = 1;
		setsockopt(skt, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval));

		/* 设为非阻塞模式 */
		ioctl(skt, FIONBIO, (int*)&unblock);
		err1 = errno;
		
		if (connect(skt, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == 0)
		{
			LOGD("connect success!!!");
			break;
		}
		err2 = errno;
		if (err2 == EINPROGRESS)
		{
			/* 判断连接是否成功 */
			n = 0;
WAIT:
			FD_ZERO(&rset);
			FD_SET(skt, &rset);
			wset = rset;

			timeVal.tv_sec = 5;
			timeVal.tv_usec = 0;
			ret = select(skt + 1, &rset, &wset, NULL, &timeVal);

			if (ret > 0)
			{
				/* 连接成功，则发送成功消息 */
				if (FD_ISSET(skt, &rset))
				{
					LOGI("connect success,and send massage");
					char buf[16];
					int len = recv(skt, buf, sizeof(buf), 0);
					err1 = errno;
					if (len == -1 && (err1 == EWOULDBLOCK || err1 == EINPROGRESS))
					{

						if (++n < 3)
						{

							goto WAIT;
						}
					}
				}
				else if (FD_ISSET(skt, &wset))
				{

					int error = 0;
					socklen_t len = sizeof(error);
					if (getsockopt(skt, SOL_SOCKET, SO_ERROR, (void *) &error, &len) == 0 && !error)
					{
						break;
					}
				}
			}
		}

		close(skt);
		skt = -1;

		//将当前ip添加到失败表中
		LOGD("[RTSP](%s) connect FAILED, add to FAIL_TABLE!\n", __FUNCTION__);
		rtsp_failedrrs_add(*((unsigned long *)&sockaddr.sin_addr), 0);
		//将当前ip添加到skip表中
		rtsp_failedrrs_add(*((unsigned long *)&sockaddr.sin_addr), 1);

NEXT_RRS:
		LOGD("33333333");
		if (mBackupNum > 0)
		{

			if (i < mBackupNum)
			{

				LOGI("[RTSP] (%s) connect use backup %s:%d\n", __FUNCTION__,
					 mBackupServers[i], mBackupPorts[i]);
				serverip = inet_addr(mBackupServers[i]);
				memset(&sockaddr, 0, sizeof(sockaddr));
				sockaddr.sin_family = AF_INET;
				sockaddr.sin_port = htons(mBackupPorts[i]);
				sockaddr.sin_addr = *(struct in_addr *)(&serverip);
			}
			else
			{

				i++;
				break;
			}
		}
	}
	LOGD("44444");
	if ((mBackupNum == 0 && i >= 2) || (mBackupNum > 0 && i > mBackupNum))
	{

		if (passtime == 0)
		{

			//至此所有不在失败表中的ip都已经在skip表中, 修改passtime使用skip表即可再次尝试所有开始在failed表中而未尝试的rrs
			passtime++;
			//从main_rrsip开始
			serverip = inet_addr(mServerName);
			memset(&sockaddr, 0, sizeof(sockaddr));
			sockaddr.sin_family = AF_INET;
			sockaddr.sin_port = htons(mPort);
			sockaddr.sin_addr = *(struct in_addr *)(&serverip);
			LOGI("[RTSP] (%s) tried all nofailed rrs, retry all nochecked!\n", __FUNCTION__);
			goto RE_CHECK;
		}
		LOGE("[RTSP] (%s) rtsp create socket error! skt=%d\n", __FUNCTION__, skt);
		return -1;
	}
	else
	{

		//如果用备份的rrs登陆成功,修改播放的url
		if (mBackupNum > 0  &&  i <= mBackupNum &&  i > 0)
		{

			char *p = NULL, *q = NULL;
			char  temp[32];
			memset(temp, 0, sizeof(temp));
			sprintf(temp, "%s:%d", mBackupServers[i-1], mBackupPorts[i-1]);
			p = strstr(mUrl, "//");
			if (p != NULL)
			{

				p += strlen("//");
				q = strchr(p, '/');
			}
			if (p != NULL && q != NULL)
			{

				if ((q - p) != (int)strlen(temp))
				{

					memmove(p + strlen(temp), q, mUrl + strlen(mUrl) - q + 1);
				}
				strncpy(p, temp, strlen(temp));
			}
		}
		LOGD("set context->mServerSocket,555555555");
		mContext->mServerSocket = skt;
		return 0;
	}

}
unsigned long *WKRtsp::rtsp_failedrrs_query(unsigned long ip, int flag)
{

	unsigned long *p;
	p = (flag == 0 ? m_rrs_failed : m_rrs_skiped);
	for (int i = 0; i < MAX_RRS_FAILED; i++)
	{
		if (p[i] == ip)
		{
			return p + i;
		}
	}
	return NULL;
}
unsigned long *WKRtsp::rtsp_failedrrs_add(unsigned long ip, int flag)
{

	unsigned long *p;
	LOGI("[RTSP](%s) %s, flag:%d\n", __FUNCTION__, inet_ntoa(*((struct in_addr *)(&ip))), flag);
	int i, n;
	n = -1;
	p = (flag == 0 ? m_rrs_failed : m_rrs_skiped);
	for (i = 0; i < MAX_RRS_FAILED; i++)
	{

		if (p[i] == 0)
		{

			if (n == -1)
			{

				n = i;
			}
		}
		else if (p[i] == ip)
		{

			return p + i;
		}
	}
	if (n >= 0)
	{

		p[n] = ip;
		return p + i;
	}
	LOGI("[RTSP](%s) FAILED!\n", __FUNCTION__);
	//TODO clear all?
	return NULL;
}
bool WKRtsp::sendCommand(const char* cmd)
{

	fd_set wfds, efds;
	int i = 0;
	int ret = 0;
	int buflen = strlen(cmd);
	if (mContext->mServerSocket < 0)
		return false;
	struct timeval  timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
//	LOGD("rtsp send cmd = %s\n", cmd); 
	while (i < buflen)
	{

		/* 检查是否有数据可读 */
		FD_ZERO(&wfds);
		FD_SET(mContext->mServerSocket, &wfds);
		FD_ZERO(&efds);
		FD_SET(mContext->mServerSocket, &efds);
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;
		ret = select(mContext->mServerSocket + 1, NULL, &wfds, &efds, &timeVal);
		if (0 < ret && FD_ISSET(mContext->mServerSocket, &wfds) &&  !FD_ISSET(mContext->mServerSocket, &efds))
		{
			ret = send(mContext->mServerSocket, cmd + i, buflen - i, 0);
		}
        
		if (mContext->mServerSocket != -1)
		{
			if (ret <= 0 || FD_ISSET(mContext->mServerSocket, &efds))
			{
				LOGE("server closed socket");
				return false;
			}
		}
		i += ret;
	}
	return true;
}

int WKRtsp::rtsp_recv_data(char *buf, int size, int timeout)
{

	if (mContext->mServerSocket < 0)
	{
		LOGE("[%s]%d ===============\n", __FUNCTION__ , __LINE__);
		// TODO
		// 错误上层如何处理？
		WKHandler* h = mContext->mRtspHandler;
		h->sendMessage(h->obainMessage(WK_RTSP_SOCKET_ERROR));
		mContext->mRtspThreadState = WK_THREAD_STATE_EXITING;
		return -1;
	}

	struct pollfd pfd;

	 pfd.fd = mContext->mServerSocket;
	 pfd.events = POLLIN;
	 //while(1)
	 {
	 	int retval = poll(&pfd,1,timeout);
	   
		if(retval < 0)
		{
			LOGE("poll: %s/n",strerror(errno));
			return -1;
		}
		if(retval == 0)
		{//timeout
			return 0;
		} 
		if(((pfd.revents&POLLHUP) == POLLHUP) ||
		((pfd.revents&POLLERR) == POLLERR) ||
		((pfd.revents&POLLNVAL) == POLLNVAL))
		{
			LOGE("Socket closed");
			if (mContext->mRtspThreadState == WK_THREAD_STATE_RUNNING) // running的时候出现这种情况属异常，其他情况可能是正常退出
			{
				LOGE("RTSP Socket error  %d !!!!!!!!!!!!!!!!!", errno);
				WKHandler* h = mContext->mRtspHandler;
				h->sendMessage(h->obainMessage(WK_RTSP_SOCKET_ERROR));
				mContext->mRtspThreadState = WK_THREAD_STATE_EXITING;
			}
			return -1;
		}
		
		if((pfd.revents & POLLIN) == POLLIN)
		{
			int recvsize = 0;
			recvsize = recv(mContext->mServerSocket, buf, size, 0);
			if (recvsize > 0)
			{
				return recvsize;
			}
			
			usleep(40000);
			LOGE("Socket Shutdown by remote, exit");
			return recvsize;
		}
	 }

	return -1;
}
void WKRtsp::dispatchMessage(WKMessage* msg)
{
	switch (msg->what)
	{
	case WK_RTSP_RECEIVE_START:
	{
		LOGD("recive thread start msg!!!");
		mContext->mRtspThreadState = WK_THREAD_STATE_RUNNING;
		looper_thread();
		break;
	}
	default:
		LOGI("do nothing");
		break;
	}
}

#define RECVBUFFER_LENGTH 3000

void WKRtsp::looper_thread()
{
	char* pRTPBuf;
	unsigned long nRTPBufLength = RECVBUFFER_LENGTH;
	char prefix[256];
	unsigned int  i_prefix = 0;
	int size;
	int next_read_size;
	int cur_read_size; 
    unsigned seq = 0xFFFFFFFF;
//#if defined(SK_SUPPORT_LN_CU)
	int header_read_size = 0;
	int header_left_size = 0;
	char header[256] = {0};
	int berrorpackage = 0;
	bool has_recvd_rtp = false;
//#endif

	LOGE("RTSP receive thread start %d!!!",gettid());
	nice(-40);
    WKMediaProcessor* processor = sk_mc_get_processor();
    mIsThreadRunning = true;

	pRTPBuf = (char *)malloc(RECVBUFFER_LENGTH);
	if(pRTPBuf == NULL)
	{
		LOGE("RTSP receive thread malloc failed");
		return;
	}
	// while(mContext->mRtspThreadState == SK_THREAD_STATE_RUNNING)
	while (/*(mContext->getReceiveThreadState() == SK_THREAD_STATE_RUNNING)
        && */(mContext->mRtspThreadState == WK_THREAD_STATE_RUNNING)
        && ((mContext->getStreamType() & WK_STREAM_MULTICAST) == 0))
	{
		//接收四个字节的头
		//LOGD("rtsp thread runing...");

		int size = rtsp_recv_data(prefix + i_prefix, 4 - i_prefix, 500 /* 1 * 1000*/);
		if (size <= 0 || mContext->mServerSocket == -1)
		{
			continue;
		}
		else
		{
			i_prefix += size;
			if (i_prefix < 4)
			{
				LOGD(" ok ! size:%d i_prefix:%d \n", size, i_prefix);
				continue;
			}
		}

		//RTP/RTCP数据
		if (prefix[0] == '$'  && prefix[1] < 4)
		{
			i_prefix = 0;
			// chnl_id = prefix[1];
			next_read_size = prefix[2];
			next_read_size = (next_read_size << 8) + prefix[3];
			if(next_read_size > nRTPBufLength)
			{
				free(pRTPBuf);
				nRTPBufLength = next_read_size;
				pRTPBuf = (char *)malloc(nRTPBufLength);
				if(pRTPBuf == NULL)
				{
					LOGE("RTSP receive thread malloc failed");
					return;
				}
			}

			// TODO:如果网络出了问题，读不到想要的数据，不判断退出状态循环就会死在这里
			// TODO:处理不完整的TS帧
			// 读取这一TCP帧的数据
			int pkg_read_size = 0;
            int skip_size = 0;
            
            //LOGD("next_read_size:%d", next_read_size);
			while ((next_read_size > 0)
                       && (mContext->mRtspThreadState == WK_THREAD_STATE_RUNNING))
			{
				size = rtsp_recv_data(pRTPBuf + pkg_read_size, next_read_size, 1000);
				if (size < 0)
				{
					LOGE("RTSP recive err!");
					break;
				}
				pkg_read_size += size;
				next_read_size -= size;
			}
            //LOGD("pkg_read_size:%d", pkg_read_size);
            
            if(pRTPBuf[0] != 0x47)
            {
                RtpPackageHeader *rtpHeader = (RtpPackageHeader*)pRTPBuf;
                unsigned int extn_len = 0;
    
        		if (rtpHeader->ph_v == 2 && (12 + (4 * rtpHeader->ph_cc) <= pkg_read_size))
        		{
        			skip_size = (12 + (rtpHeader->ph_cc * 4));
        			if (rtpHeader->ph_x)
        			{
        				extn_len = (pRTPBuf[2+skip_size] << 8) | pRTPBuf[3+skip_size];
        				skip_size += (extn_len + 1) * 4;
        			}

                    unsigned short usseq = ntohs(rtpHeader->ph_seq);
					unsigned int timestamp = ntohl(rtpHeader->ph_ts);


#if defined(WK_SUPPORT_QOS)		
		        	{
		        	   	int r_size = pkg_read_size - skip_size;
						if(!has_recvd_rtp)
        				{
            				LOGD("received first rtp packet, seq:%u", usseq);
            				has_recvd_rtp = true;
							mContext->mSkQosStatistics->recordFirstMediaData();
        				}
						  
						mContext->mSkQosStatistics->setRTPRecvData(pRTPBuf+skip_size , r_size , usseq , timestamp);
		        	}
#endif
        		}
        		else
        		{
        			if(strncmp(pRTPBuf , "ZXV" , 3) == 0)
        			{
        				//LOGD("receive Server NAT packet \n");
        			}
        			else
        			{
        				LOGD(" (%s) ph_v=%d rtpHeader->ph_cc=%d size=%d buf[0]:%x \n", __FUNCTION__, rtpHeader->ph_v, rtpHeader->ph_cc, pkg_read_size, pRTPBuf[0]);
        			}

                    continue;
        		}
            }

            if (skip_size >= size || skip_size < 0)
        	{
        		LOGE(" (%s) Invalid skip_size %d\n", __FUNCTION__, skip_size);
        		skip_size = 0;
        		continue;
        	}

            //LOGD("tcp rtp skip_size:%d", skip_size);
            char* write_buf = pRTPBuf + skip_size;
            int write_size = pkg_read_size - skip_size;
            
			berrorpackage = 0;
			for(int i = 0; i < write_size; i += 188)
			{
				if(write_buf[i] != 0x47)
				{
					LOGE("======================sync byte error %d %d", i, write_buf[i]);
					berrorpackage = 1;
				}
			}
			if(!berrorpackage)
			{
                //LOGD("write buf to processor, write_buf[0]:%d, size:%d, header_read_size:%d", write_buf[0], write_size, header_read_size);
				processor->InputDataMediaProcessor(write_buf + header_read_size, write_size, seq);
			}
		}
		//RTSP response
		else if (strncasecmp((char *)prefix, "RTSP", 4) == 0
				 || strncasecmp((char *) prefix, "SET_PARAMETER", 4) == 0
				 || strncasecmp((char *)prefix, "REDIRECT", 4) == 0
				 || strncasecmp((char *)prefix, "ANNOUNCE", 4) == 0)
		{
			i_prefix = 0;
			if (!processResponse(prefix))
			{
				// TODO:
				LOGE("Socket Error , exit receive thread. response");
				break;
			}
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{

				prefix[i] = prefix[i+1];
			}
			i_prefix = 3;
		}
	}
	//mContext->mRtspThreadState = SK_THREAD_STATE_EXITED;
	free(pRTPBuf);
	LOGE("rtsp recive thread exit!!!");
    mIsThreadRunning = false;
	return ;
}
bool WKRtsp::processTsPackage(const char* prefix)
{
	LOGI("process ts package");
	return true;
}

#define RTSP_RECV_BUF_SIZE (16 * 1024)

bool WKRtsp::processResponse(const char* prefix)
{
	char rcvbuf[RTSP_RECV_BUF_SIZE];
	memset(rcvbuf, 0, sizeof(rcvbuf));
	memcpy(rcvbuf, prefix, 4);
	char *p = rcvbuf + 4;
	WKHandler* h = mContext->mRtspHandler;
	// rtsp命令的响应并不是同步的，典型的getParmater是随机的

	//接收RTSP响应的头部
	unsigned int i = 0;
	for (i = 4; i < sizeof(rcvbuf); i++)
	{
		if (rtsp_recv_data(p, 1, 1 * 1000) <= 0)
		{
            LOGE("processResponse, rtsp_recv_data err.");
			break;
		}
		if (*(p - 3) == '\r' && *(p - 2) == '\n' && *(p - 1) == '\r' && *p == '\n')
		{
			i++;
			p++;
			break;
		}
		p++;
	}

    LOGE("processResponse, i:%d", i);
    if(i >= RTSP_RECV_BUF_SIZE)
	{
		i = RTSP_RECV_BUF_SIZE - 1;
	}

	rcvbuf[i] = '\0';
	LOGD("headerbegin!!!!%s!!!!end", rcvbuf);
	if (strncasecmp(rcvbuf, "ANNOUNCE", 4) == 0)
	{
		h->sendMessage(h->obainMessage(WK_RTSP_CTRL_ANNOUNCE, 0, 0, strdup(rcvbuf)));
		return true;
	}
	WKRtspResponseDecoder* response = new WKRtspResponseDecoder();
	//分析RTSP响应头部
	if (!response->decode(rcvbuf))
	{
		LOGI("response decode error!");
		return true;
	}

	//接收RTSP负载
	if (response->content_length > 0)
	{
		// TODO:所有malloc的地方都需要判断内存是否足够
		response->body = (char*)malloc(response->content_length + 1);
		memset(response->body, 0, response->content_length + 1);
		int len = 0;
		while (len < response->content_length)
		{
            //LOGD("recv 2....");
			int size = rtsp_recv_data((char *)(response->body + len), response->content_length - len, 1000);
			if (size < 0)
			{

				break;
			}
			len += size;
		}
		response->body[len] = '\0';
		LOGD("RTSP Content: [%s]\n", response->body);
		// 解析SDP
		response->decodesdp(response->body);
	}

    //通知外面接受到应答
	//response->mHoldSelf->incStrong(response->mHoldSelf);
	if (strncasecmp(rcvbuf, "RTSP", 4) == 0)
	{
		h->sendMessage(h->obainMessage(WK_RTSP_CTRL_RESPONSE, 0, 0, response->mHoldSelf));
	}
	else
	{
		LOGE("response not been handler!!!");
	}
    
	//如果服务器已经断开,则关闭
	if (response->close_connection && response->retcode[0] != '3') //<tempctc> && response->retcode[0] != '3'
	{
		LOGE("[RTSP] (%s) server closed !\n", __FUNCTION__);
		if (mContext->mServerSocket != -1)
		{

			close(mContext->mServerSocket);
			mContext->mServerSocket = -1;
		}
		return false;
	}
	return true;
}
void WKRtsp::stop()
{
	// TODO
	if(!mIsThreadRunning)
		return;
	mContext->mRtspThreadState = WK_THREAD_STATE_EXITING;
	LOGE("rtsp recv thread is exiting!");

	//select timeout is 1seconds , to force recv exit , force close socket. 
	if(mContext->mServerSocket > 0)
	{
		LOGD("Close socket!");
		shutdown(mContext->mServerSocket,SHUT_RDWR);

		//add by lgl
		close(mContext->mServerSocket);
	}
    mContext->mServerSocket = -1;

    while(mIsThreadRunning){
        usleep(100);
    }

    LOGE("rtsp recv thread is exit!");
}
