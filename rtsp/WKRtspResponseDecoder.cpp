#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include "wk_android_std.h"
#include "WKRtspResponseDecoder.h"

#include <String.h>
#include <stdlib.h>
#include <string.h>



#define MODULE_NAME "WK_RTSP_RESPONSE_DECODER"

#define ADV_SPACE(a) {while (isspace(*(a)) && (*(a) != '\0'))(a)++;}

static const char *SPACES = " \t";

using namespace android;

static bool get_next_line(char **polptr, const char **decode, /*U32*/int *buflen)
{
	char *fret;
	/*U32*/
	int len;
	unsigned int buflen_left;
	const char *cptr;

	cptr = *decode;

	if(*cptr == '\0')
	{
		return false;
	}

	while(*cptr != '\0' && *cptr != '\n' && *cptr != '\r')
	{
		cptr++;
	}

	len = cptr - *decode;
	if(*buflen <= len + 1)
	{
		if(len > 65535)
		{
			LOGE("Max line length of 65535 exceeded %u", len);
			return (false);
		}
		*polptr = (char*)realloc(*polptr, len + 1);
		*buflen = len + 1;
	}
	memcpy(*polptr, *decode, len);
	(*polptr)[len] = '\0';
	*decode += len;
	while(*(*decode) == '\n' || *(*decode) == '\r')
	{
		(*decode)++;
	}
	
	return true;
}

/*
* sdp_decode_parse_media()
* decodes m= lines.
* m=<media> <port>[/<numport>] <proto> <fmt list>
* Inputs:
*   lptr - pointer to line
*   sptr - pointer to session description to modify
* Outputs:
*   pointer to new media description
*/
static bool sdp_decode_parse_media(const char *buf, WKRtspResponseDecoder *decoder)
{
	char *mdesc, *proto, *sep;
	unsigned int read_in, port_no;
    char *lptr = NULL;

    if(buf != NULL)
        lptr = const_cast<char*>(buf);
    
	// m=<media> <port> <transport> <fmt list>
	mdesc = strsep(&lptr, SPACES);
	if(mdesc == NULL || lptr == NULL)
	{
		LOGE("No media type");
		return false;
	}

	// <port>
	ADV_SPACE(lptr);
	read_in = 0;
	if(!isdigit(*lptr))
	{
		LOGE("Illegal port number in media %s", lptr);
		return false;
	}
    
	while(isdigit(*lptr))
	{
		read_in *= 10;
		read_in += *lptr - '0';
		lptr++;
	}
	ADV_SPACE(lptr);

	// number of ports (optional)
	if(*lptr == '/')
	{
		lptr++;
		ADV_SPACE(lptr);
		if(!isdigit(*lptr))
		{
			LOGE("Illegal port number in media %s", lptr);
			return false;
		}
		sep = strsep(&lptr, SPACES);
		if(lptr == NULL)
		{
			LOGE("Missing keywords in media");
			return false;
		}
		sscanf(sep, "%u", &port_no);
		ADV_SPACE(lptr);
	}
	else
	{
		port_no = 0;
	}

	// <transport> (protocol)
	proto = strsep(&lptr, SPACES);
	if(proto == NULL || lptr == NULL)
	{
		LOGE("No transport in media");
		return false;
	}
	ADV_SPACE(lptr);
	if(!isalnum(*lptr))
	{
		return false;
	}

    decoder->sdp_media_port = read_in;
#if 0
	// malloc memory and set.
	new_meida = malloc(sizeof(media_desc_t));
	if(new_meida == NULL)
	{
		*err = SK_FAILED;
		return (NULL);
	}
	memset(new_meida, 0, sizeof(media_desc_t));
	new_meida->media = strdup(mdesc);
	new_meida->port = (uint16_t)read_in;
	new_meida->proto = strdup(proto);
	new_meida->num_ports = (unsigned short)port_no;

	// parse format list - these are not necessarilly lists of numbers
	// so we store as strings.
	q = NULL;
	do
	{
		sep = strsep(&lptr, SPACES);
		if(sep != NULL)
		{
			if(sdp_add_format_to_list(new_meida, sep) == NULL)
			{
				free_media_desc(new_meida);
				*err = SK_FAILED;
				return (NULL);
			}
			if(lptr != NULL)
			{
				ADV_SPACE(lptr);
			}
		}
	}
	while(sep != NULL);

	new_meida->parent = sptr;
	// Add to list of media
	if(sptr->media == NULL)
	{
		sptr->media = new_meida;
	}
	else
	{
		mp = sptr->media;
		while(mp->next != NULL)
		{
			mp = mp->next;
		}
		mp->next = new_meida;
	}

	return (new_meida);
#endif
    return true;
}

/*
* sdp_decode_parse_connect()
* parse c=<network type> <address type> <connect address>
* Inputs: lptr, connect pointer
* Outputs - error code or 0 if parsed correctly
*/
static bool sdp_decode_parse_connect(const char *buf, WKRtspResponseDecoder *decoder)
{
	char *sep, *beg;
    char *lptr = NULL;

    if(buf == NULL)
        return false;

    lptr = const_cast<char*>(buf);

	decoder->sdp_connect_ttl = 0;
	decoder->sdp_connect_num_addr = 0;

	// sw_log_info("[sdp_decode_parse_connect] lptr=%s\n", lptr );
	// <network type> should be IN
	sep = strsep(&lptr, SPACES);
	if(sep == NULL || lptr == NULL || strcasecmp(sep, "IN") != 0)
	{
		LOGE("IN statement missing from c");
		return false;
	}

	// <address type> - should be IPV4
	ADV_SPACE(lptr);
	sep = strsep(&lptr, SPACES);
	if(sep == NULL || lptr == NULL)
	{
		LOGE("No connection type in c=");
		return false;
	}
	//cptr->conn_type = strdup(sep);//IP4

	// Address - first look if we have a / - that indicates multicast, and a
	// ttl.
	ADV_SPACE(lptr);
    LOGD("check /, now lptr:%s", lptr);
	sep = strchr(lptr, '/');
	if(sep == NULL)
	{
		// unicast address
		decoder->sdp_connect_addr = strdup(lptr);//取到地址
		return true;
	}
    LOGD("check /, sep not null, sep:%s", sep);
	// Okay - multicast address.  Take address up to / (get rid of trailing spaces)
	beg = lptr;
	lptr = sep + 1;
	sep--;
	while(isspace(*sep) && sep > beg)
	{
		sep--;
	}
	sep++;
	*sep = '\0';
	decoder->sdp_connect_addr = strdup(beg);

	// Now grab the ttl
	ADV_SPACE(lptr);
	sep = strsep(&lptr, " \t/");
	if(!isdigit(*sep))
	{
		decoder->free_safe(decoder->sdp_connect_addr);
		decoder->sdp_connect_addr = NULL;

		LOGE("No multicast TTL in c=, sep:%s", sep);
		return false;
	}
	sscanf(sep, "%u", &(decoder->sdp_connect_ttl));

	return true;
}

WKRtspResponseDecoder::WKRtspResponseDecoder()
{
	content_length = 0;
	cseq = 0;
	close_connection = false;
	memset(retcode, 0, sizeof(retcode));         /* 3 byte return code - \0 delimited */
	retresp = NULL;
	body = NULL;              /* Contains body returned */
	accept = NULL;
	accept_encoding = NULL;
	accept_language = NULL;
	allow_public = NULL;
	authorization = NULL;
	bandwidth = NULL;
	blocksize = NULL;
	cache_control = NULL;
	content_base = NULL;
	content_encoding = NULL;
	content_language = NULL;
	content_location = NULL;
	content_type = NULL;
	cookie = NULL;
	date = NULL;
	expires = NULL;
	from = NULL;
	if_modified_since = NULL;
	last_modified = NULL;
	location = NULL;
	proxy_authenticate = NULL;
	proxy_require = NULL;
	range = NULL;
	referer = NULL;
	require = NULL;
	retry_after = NULL;
	rtp_info = NULL;
	scale = NULL;
	server = NULL;
	session = NULL;
	speed = NULL;
	transport = NULL;
	unsupported = NULL;
	user_agent = NULL;
	via = NULL;
	www_authenticate = NULL;
	time_shift = NULL;
	timeshift_status = NULL;
	time_current = NULL;
	x_Burst = NULL;
	x_Retrans = NULL;
	sdp_connect_ttl = 0;
	sdp_connect_addr = NULL;
	sdp_connect_num_addr = 0;
	sdp_media_port = 0;

	next = NULL;
	rangeStart = 0;
	rangeEnd = 0;
	mStreamType.clear();
	mHoldSelf = this;
}
WKRtspResponseDecoder::~WKRtspResponseDecoder()
{
	// free char*
	free_safe(retresp);
	free_safe(body);
	free_safe(accept);
	free_safe(accept_encoding);
	free_safe(accept_language);
	free_safe(allow_public);
	free_safe(authorization);
	free_safe(bandwidth);
	free_safe(blocksize);
	free_safe(cache_control);
	free_safe(content_base);
	free_safe(content_encoding);
	free_safe(content_language);
	free_safe(content_location);
	free_safe(content_type);
	free_safe(cookie);
	free_safe(date);
	free_safe(expires);
	free_safe(from);
	free_safe(if_modified_since);
	free_safe(last_modified);
	free_safe(location);
	free_safe(proxy_authenticate);
	free_safe(proxy_require);
	free_safe(range);
	free_safe(referer);
	free_safe(require);
	free_safe(retry_after);
	free_safe(rtp_info);
	free_safe(scale);
	free_safe(server);
	free_safe(session);
	free_safe(speed);
	free_safe(transport);
	free_safe(unsupported);
	free_safe(user_agent);
	free_safe(via);
	free_safe(www_authenticate);
	free_safe(time_shift);
	free_safe(timeshift_status);
	free_safe(time_current);
	free_safe(x_Burst);
	free_safe(x_Retrans);
	free_safe(sdp_connect_addr);
	next = NULL;
}
void WKRtspResponseDecoder::free_safe(void *ptr)
{
	if(ptr != NULL)
		free(ptr);
}
void WKRtspResponseDecoder::dump()
{
}
const char *WKRtspResponseDecoder::find_seperator(const char *ptr)
{

	static const char *end2 = "\n\r";
	static const char *end1 = "\r\n";
	static const char *end3 = "\r";
	static const char *end4 = "\n";

	while (*ptr != '\0')
	{

		if (*ptr == '\r')
		{

			if (*(ptr + 1) == '\n')
			{

				return (end1);
			}
			else
			{

				return (end3);
			}
		}
		else if (*ptr == '\n')
		{

			if (*(ptr + 1) == '\r')
			{

				return (end2);
			}
			else
			{

				return (end4);
			}
		}
		ptr++;
	}
	return (NULL);
}
void WKRtspResponseDecoder::dec_dup_warn(char **location, const char *lptr)
{

	int i = 0, s = -1, e = -1;

	//找到NAME的结束位置
	while (lptr[i] != '\0' && lptr[i] != '\r' && lptr[i] != '\n')
	{

		if (lptr[i] == ':' || lptr[i] == ' ')
		{

			break;
		}
		i++;
	}
	//跳过无效字符,找到VALUE的开始位置
	while (lptr[i] != '\0' && lptr[i] != '\r' && lptr[i] != '\n')
	{

		if (lptr[i] != ' ' && lptr[i] != ':')
		{

			break;
		}
		i++;
	}
	s = i;

	//找到VALUE的结束位置
	while (lptr[i] != '\0' && lptr[i] != '\r' && lptr[i] != '\n')
	{

		i++;
	}
	e = i;

	if (*location == NULL)
	{

		*location = (char*)calloc(e - s + 1, 1);
	}
	else
	{

		*location = (char*)realloc(*location, e - s + 1);
	}
	strncpy(*location, lptr + s, e - s);
	(*location)[e-s] = 0;
}
void WKRtspResponseDecoder::rtsp_decode_header(const char *lptr)
{

	char *value = NULL;

	if (strncasecmp(lptr, "AlLow", 5) == 0)
	{

		dec_dup_warn(&allow_public, lptr);
	}
	else if (strncasecmp(lptr, "Public", 6) == 0)
	{

		dec_dup_warn(&allow_public, lptr);
	}
	else if (strncasecmp(lptr, "Connection", 10) == 0)
	{

		dec_dup_warn(&value, lptr);
		if (strncasecmp(value, "close", 5) == 0)
		{

			close_connection = true;
		}
	}
	else if (strncasecmp(lptr, "cookie", 6) == 0)
	{

		dec_dup_warn(&cookie, lptr);
	}
	else if (strncasecmp(lptr, "Content-Base", 12) == 0)
	{

		dec_dup_warn(&content_base, lptr);
	}
	else if (strncasecmp(lptr, "Content-Length", 14) == 0)
	{

		dec_dup_warn(&value, lptr);
		content_length = strtoul(value, NULL, 10);
	}
	else if (strncasecmp(lptr, "Content-Location", 16) == 0)
	{

		dec_dup_warn(&content_location, lptr);
	}
	else if (strncasecmp(lptr, "Content-Type", 12) == 0)
	{

		dec_dup_warn(&content_type, lptr);
	}
	else if (strncasecmp(lptr, "CSeq", 4) == 0)
	{

		dec_dup_warn(&value, lptr);
		cseq = strtoul(value, NULL, 10);
	}
	else if (strncasecmp(lptr, "Location", 8) == 0)
	{

		dec_dup_warn(&location, lptr);
	}
	else if (strncasecmp(lptr, "Range", 5) == 0)
	{

		dec_dup_warn(&range, lptr);
	}
	else if (strncasecmp(lptr, "x_time_shift_range", 18) == 0)
	{

		dec_dup_warn(&range, lptr);
	}
	else if (strncasecmp(lptr, "Retry-After", 11) == 0)
	{

		dec_dup_warn(&retry_after, lptr);
	}
	else if (strncasecmp(lptr, "RTP-Info", 8) == 0)
	{

		dec_dup_warn(&rtp_info, lptr);
	}
	else if (strncasecmp(lptr, "Session", 7) == 0)
	{

		dec_dup_warn(&session, lptr);
	}
	else if (strncasecmp(lptr, "Set-Cookie", 10) == 0)
	{

		dec_dup_warn(&location, lptr);
	}
	else if (strncasecmp(lptr, "Speed", 5) == 0)
	{

		dec_dup_warn(&speed, lptr);
	}
	else if (strncasecmp(lptr, "Transport", 9) == 0)
	{

		dec_dup_warn(&transport, lptr);
	}
	else if (strncasecmp(lptr, "WWW-Authenticate", 16) == 0)
	{

		dec_dup_warn(&www_authenticate, lptr);
	}
	else if (strncasecmp(lptr, "Accept", 6) == 0)
	{

		dec_dup_warn(&accept, lptr);
	}
	else if (strncasecmp(lptr, "Accept-Encoding", 15) == 0)
	{

		dec_dup_warn(&accept_encoding, lptr);
	}
	else if (strncasecmp(lptr, "Accept-Language", 15) == 0)
	{

		dec_dup_warn(&accept_language, lptr);
	}
	else if (strncasecmp(lptr, "Authorization", 13) == 0)
	{

		dec_dup_warn(&authorization, lptr);
	}
	else if (strncasecmp(lptr, "Bandwidth", 9) == 0)
	{

		dec_dup_warn(&bandwidth, lptr);
	}
	else if (strncasecmp(lptr, "Blocksize", 9) == 0)
	{

		dec_dup_warn(&blocksize, lptr);
	}
	else if (strncasecmp(lptr, "Cache-Control", 13) == 0)
	{

		dec_dup_warn(&cache_control, lptr);
	}
	else if (strncasecmp(lptr, "Date", 4) == 0)
	{

		dec_dup_warn(&date, lptr);
	}
	else if (strncasecmp(lptr, "Expires", 7) == 0)
	{

		dec_dup_warn(&expires, lptr);
	}
	else if (strncasecmp(lptr, "From", 4) == 0)
	{

		dec_dup_warn(&from, lptr);
	}
	else if (strncasecmp(lptr, "If-Modified-Since", 17) == 0)
	{

		dec_dup_warn(&if_modified_since, lptr);
	}
	else if (strncasecmp(lptr, "Last-Modified", 13) == 0)
	{

		dec_dup_warn(&last_modified, lptr);
	}
	else if (strncasecmp(lptr, "Proxy-Authenticate", 18) == 0)
	{

		dec_dup_warn(&proxy_authenticate, lptr);
	}
	else if (strncasecmp(lptr, "Proxy-Require", 13) == 0)
	{

		dec_dup_warn(&proxy_require, lptr);
	}
	else if (strncasecmp(lptr, "Referer", 7) == 0)
	{

		dec_dup_warn(&referer, lptr);
	}
	else if (strncasecmp(lptr, "Scale", 5) == 0)
	{

		dec_dup_warn(&scale, lptr);
	}
	else if (strncasecmp(lptr, "Server", 6) == 0)
	{

		dec_dup_warn(&server, lptr);
	}
	else if (strncasecmp(lptr, "Unsupported", 11) == 0)
	{

		dec_dup_warn(&unsupported, lptr);
	}
	else if (strncasecmp(lptr, "User-Agent", 10) == 0)
	{

		dec_dup_warn(&user_agent, lptr);
	}
	else if (strncasecmp(lptr, "Via", 3) == 0)
	{

		dec_dup_warn(&via, lptr);
	}
	else if (strncasecmp(lptr, "Timeshift-Status", 16) == 0)
	{

		dec_dup_warn(&timeshift_status, lptr);
	}
	else if (strncasecmp(lptr, "x-Timeshift_Range", 17) == 0)
	{

		dec_dup_warn(&time_shift, lptr);
	}
	else if (strncasecmp(lptr, "x-Timeshift_Current", 19) == 0)
	{

		dec_dup_warn(&time_current, lptr);
	}
 	else if (strncasecmp(lptr, "x-Burst", 7) == 0)
	{
		dec_dup_warn(&x_Burst, lptr);
	}
	else if(strncasecmp(lptr, "x-Retrans", 9) == 0)
	{
		dec_dup_warn(&x_Retrans, lptr);
	}
	else
	{
		LOGI("can not identify:%s", lptr);
	}

	if (value)
	{

		free(value);
	}
}
bool WKRtspResponseDecoder::decode(const char* buf)
{
	const char *seperator = NULL;
	const char *lptr = NULL, *p = NULL;
	int  cnt = 0;

	//寻找换行符
	seperator = find_seperator(buf);
	if (seperator == NULL)
	{
		LOGE("bad response!");
		return false;
	}

	//第一行应该是"RTSP/1.0 200 OK" 或 ...
	if (strncasecmp(buf, "RTSP/1.0", strlen("RTSP/1.0")) != 0)
	{
		LOGE("bad reponse header!");
		return false;
	}

	p =  buf + strlen("RTSP/1.0");
	// ADV_SPACE(p);
	while (isspace(*p) && (*p != '\0'))
		p++;
	switch (*p)
	{

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		break;
	default:
		LOGE("response wrong number");
		return false;
	}

	memcpy(retcode, p, 3);
	retcode[3] = '\0';

	p += 3;
	while (isspace(*p) && (*p != '\0'))
		p++;
	if (*p != '\0' && *p != '\r' && *p != '\n')
	{

		int n = 0;
		char tmp[256];
		while (p[n] != '\0' && p[n] != '\r' && p[n] != '\n' && n < (int)sizeof(tmp) - 1)
		{

			tmp[n] = p[n];
			n++;
		}
		tmp[n] = '\0';
		retresp = strdup(tmp);
		// LOGI("response retresp:%s",retresp);
	}

	//分析头部字段
	lptr = p;
	//LOGE("lptr:%s",buf);
	while (true)
	{

		p = strstr(lptr, seperator);//找到换行符的位置
		if (p == NULL)
		{
			break;
		}		
		lptr = p + strlen(seperator); // 当前lptr指向retresp的值，应先跳过
		if(( *lptr != '\r') && (*lptr != '\n') &&  (*lptr != '\0'))
		{
			rtsp_decode_header(lptr);
		}
	}

	return true;
}

bool WKRtspResponseDecoder::decodesdp(const char* buf)
{
	const char *seperator = NULL;
	const char *lptr = NULL, *p = NULL, *dptr = NULL;
	char code = '\0';
	bool ret = true;
    char *line = NULL;
    int linelen = 0;

	seperator = find_seperator(buf);

	if (seperator == NULL)
	{
		LOGE("bad response!");
		return false;
	}
	//lptr = buf;
	//ADV_SPACE(lptr);
    dptr = buf;
	LOGE("SDP :%s", dptr);
	while (ret && get_next_line(&line, &dptr, &linelen))
	{
        lptr = line;
        ADV_SPACE(lptr);
        //LOGD("get line, lptr:%s", lptr);
		if(*lptr == '\0')
		{
			continue;
		}
		code = *lptr++;
		ADV_SPACE(lptr);
		if (*lptr == '=')
		{
			lptr++;
			ADV_SPACE(lptr);
			// if(tolower(code) != 'v')
			// {
			// 	LOGE("Version not 1st statement");
			// 	break;
			// }
			switch (tolower(code))
			{
			//RTSP Version
			case 'a':
			{
				//LOGD("get sdp a:%s", lptr);
				//media clip range 内容时间范围
				if (strncasecmp(lptr, "range", strlen("range")) == 0)
				{
					LOGI("sdp get range");
					lptr += strlen("range");
					ADV_SPACE(lptr);
					if (*lptr != ':')
					{
						LOGE("parse sdp range error");
					}
					lptr++;	// pase colon
					ADV_SPACE(lptr);
					if (strncasecmp(lptr, "npt", strlen("npt")) == 0)
					{
                        if(mStreamType.isEmpty())
                        {
						    mStreamType.setTo("vod");
                        }
                        
						lptr += strlen("npt");
						lptr ++;
						p = strchr(lptr, '-');
                        if(p != NULL)
                        {
    						char value[128];
    						memset(value, 0, sizeof(value));
    						strncpy(value, lptr, p - lptr);
    						LOGE("%s", value);
    						rangeStart = atoi(value);
    						memset(value, 0, sizeof(value));
    						p++;
                            ADV_SPACE(p);
    						//p = strstr(lptr, seperator);
    						strncpy(value, p, strlen(p));
    						LOGE("%s", value);
    						rangeEnd = atoi(value);
                        }
					}
					else if (strncasecmp(lptr, "clock", strlen("clock")) == 0)
					{
						LOGI("utc range,live ,TODO");
                        if(mStreamType.isEmpty())
                        {
						    mStreamType.setTo("live");
                        }
                        
						lptr += strlen("clock");
						lptr ++;
						p = strchr(lptr, '-');
                        if(p != NULL)
                        {
    						char value[128];
    						memset(value, 0, sizeof(value));
    						strncpy(value, lptr, p - lptr);
    						LOGE("%s", value);
    						rangeStart = ascutc2time(value);
    						memset(value, 0, sizeof(value));
    						p++;
    						ADV_SPACE(p);
    						//p = strstr(lptr, seperator);
    						strncpy(value, p, strlen(p));
    						LOGE("%s", value);
    						rangeEnd = ascutc2time(value);
                        }
					}
				}
				/*
				 * a=control : Media URL 流媒体地址
				 * A=framerate : Clip frame rate 帧频
				 * a=x-backupurl : backup URL 备份URL
				 * a=x-framegeometry : Clip framegeometry(width and height in pixels) 分辨率
				 * a=x-name : Name of asset or clip (depending on context) 内容名称
				 */
				else
				{
					LOGD("parse sdp a current not support, line:%s", line);
				}
				break;
			}
			/* 
			 * Media bitrate
			 * 码率
			 */
			case 'b':
			{
				break;
			}
			/* 
			 * Connection description
			 * 连接描述，可使用组播地址
			 */
			case 'c':
			{
				ret = sdp_decode_parse_connect(lptr, this);
                if(sdp_connect_addr != NULL)
                    LOGD("get sdp_connect_addr:%s", sdp_connect_addr);
				break;
			}
			/* 
			 * Email 地址
			 */
			case 'e':
			{
				break;
			}
			/* 
			 * Session or media description(depending on context)
			 * Session 的描述信息
			 */
			case 'i':
			{
				break;
			}
			/* 
			 * 加密方法和密钥
			 */
			case 'k':
			{
				break;
			}
			/* 
			 * Media description(generated, but not used)
			 * 媒体描述，需要区分是否采用RTP。即Transport 格式。
			 */
			case 'm':
			{
				ret = sdp_decode_parse_media(lptr, this);

                LOGD("get sdp_media_port:%d", sdp_media_port);
				break;
			}
			/* 
			 * Session owner
			 * Session 的创建人、版本、ID 等
			 */
			case 'o':
			{
				break;
			}
			/* 
			 * 电话号码
			 */
			case 'p':
			{
				break;
			}
			/* 
			 * 重复间隔、时长、次数
			 */
			case 'r':
			{
				break;
			}
			/* 
			 * Session name
			 * Session 名
			 */
			case 's':
			{
				mStreamType.setTo(lptr, strlen(lptr));
				break;
			}
			/* 
			 * 起始时间和结束时间
			 * Time session if active
			 */
			case 't':
			{
				break;
			}
			/* 
			 * 描述信息的URL
			 */
			case 'u':
			{
				break;
			}
			/* 
			 * RTSP version
			 * 版本
			 */
			case 'v':
			{
				// TODO
				// 第一个版本只先处理duration
				break;
			}
			/* 
			 * 时区
			 */
			case 'z':
			{
				break;
			}
			
			default:
			{
				LOGD("unkown sdp line:%s", lptr);
				break;
			}
			}
		}
		else
		{

			if (strncasecmp(lptr - 1, "TrickSpeeds", strlen("TrickSpeeds")) == 0)
			{

				LOGE("[SWMEDIA] (%s) <TrickSpeeds> found in sdp!!!!!!!!!!!!!!!!\n", __FUNCTION__);
				break;
			}
			else
			{

				// bigger than 1 character code
				// sdp_debug(LOG_ERR, "More than 1 character code - %s", line);
				LOGE("sdp unkonw line");
				ret = false;
			}
		}
	}

    if(line != NULL)
	{
		free(line);
	}
    
	return ret;
}
unsigned long WKRtspResponseDecoder::ascutc2time(const char *ascutc)
{
	struct tm t;
	if (NULL == ascutc || strlen(ascutc) < 15)
	{
		return 0;
	}
	t.tm_year = (ascutc[0] - '0') * 1000 + (ascutc[1] - '0') * 100
				+ (ascutc[2] - '0') * 10 + (ascutc[3] - '0') - 1900;
	t.tm_mon = (ascutc[4] - '0') * 10 + (ascutc[5] - '0') - 1 ;
	t.tm_mday = (ascutc[6] - '0') * 10 + (ascutc[7] - '0');
	t.tm_hour = (ascutc[9] - '0') * 10 + (ascutc[10] - '0');
	t.tm_min = (ascutc[11] - '0') * 10 + (ascutc[12] - '0');
	t.tm_sec = (ascutc[13] - '0') * 10 + (ascutc[14] - '0');

	return timegm(&t);
}
