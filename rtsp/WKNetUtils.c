#include "WKNetUtils.h"

#include "wk_android_std.h"
#include <String.h>
#include <stdlib.h>
#include <string.h>

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

#define WK_RTSP_UDP_DEFAULT_PORT 55235


bool isspace(char ch)
{
	return (ch == ' ');
}


/* 是否是有效点分十进制IP地址  */

bool is_ipaddr(short int sin_family, const char *ip_addr)
{
	int i = 0, len = 0;
	const char *p = NULL, *pSep = NULL;
	int iCount = 0;
	char szTemp[8];
	int nSect = 0;

	if (ip_addr == NULL)
	{

		return false;
	}

	len = strlen(ip_addr);
	if (len == 0 || len > 15)
	{

		return false;
	}

	for (i = 0; i < len; i++)
	{

		if ((ip_addr[i] < '0' || ip_addr[i] > '9') && (ip_addr[i] != '.'))
		{

			return false;
		}
	}

	p = ip_addr;
	while ((pSep = strchr(p, '.')) != NULL)
	{

		++iCount;

		if (pSep - p > 3)
		{

			return false;
		}

		memset((void *) szTemp, 0, sizeof(szTemp));
		strncpy(szTemp, p, pSep - p);

		nSect = atoi(szTemp);
		if (nSect > 255)
		{

			return false;
		}

		pSep++;
		p = pSep;
	}

	if (iCount != 3)
	{

		return false;
	}
	else
	{

		memset((void *) szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, p);

		nSect = atoi(szTemp);
		if (nSect > 255)
		{

			return false;
		}
	}
	return true;
}


size_t strcount(const char *string, const char *vals)
{

	size_t count = 0;
	if (string == NULL)
	{
		return 0;
	}
	while (*string != '\0')
	{
		if (strchr(vals, *string) != NULL)
		{
			count++;
		}
		string++;
	}
	return count;
}

/*为了转换空格 url中空格代表%20*/
char* convert_url(const char *to_convert)
{
	static const char *spaces = " \t";
	char *ret, *p;
	size_t count;
	if (to_convert == NULL)
	{
		return NULL;
	}
	count = strcount(to_convert, spaces);
	count *= 3; // replace each space with %20	

	count += strlen(to_convert) + 1;

	ret = (char *)malloc(count);
	p = ret;
	while (*to_convert != '\0')
	{
		if (isspace((char)(*to_convert)))
		{
			*p++ = '%';
			*p++ = '2';
			*p++ = '0';
			to_convert++;
		}
		else
		{
			*p++ = *to_convert++;
		}
	}
	*p++ = *to_convert; // final \0
	return ret;
}

U16 getRandomPort()
{
	U16 port;
	int sockfd;
    socklen_t addrlen;
   	struct sockaddr_in addr;
	//create a socket with port=0 to get random port
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
    {
    	LOGE("WKRtspCtrl randomize port failed");
    	port = WK_RTSP_UDP_DEFAULT_PORT;
    	goto error;
	}
	memset(&addr,0,sizeof(struct sockaddr_in ));
	addrlen = sizeof(struct sockaddr_in);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0); // for random port
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
	if (bind(sockfd, (struct sockaddr *)&addr,addrlen))
	{
		LOGE("WKRtspCtrl randomize port failed");
		port = WK_RTSP_UDP_DEFAULT_PORT;
		goto error;
	}
    
	memset(&addr,0,sizeof(struct sockaddr_in ));
	addrlen = sizeof(struct sockaddr_in);
	if(getsockname(sockfd, (struct sockaddr *)&addr,&addrlen))
    {
		LOGE("WKRtspCtrl randomize port failed");
		port = WK_RTSP_UDP_DEFAULT_PORT;
		goto error;
	}
	port = ntohs(addr.sin_port);
	LOGD("WKRtspCtrl port = %d",port);
error:
	if (sockfd>0)
	{
		close(sockfd);
	}
	return port;
}

