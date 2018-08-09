#ifndef H_WK_RTSP_RESPONSE_DECODER
#define H_WK_RTSP_RESPONSE_DECODER

#include <String.h>

namespace android
{
class WKRtspResponseDecoder
{
public:
	WKRtspResponseDecoder();
	virtual ~WKRtspResponseDecoder();

	int content_length;
	unsigned int cseq;
	bool close_connection;
	char retcode[4];         /* 3 byte return code - \0 delimited */
	char *retresp;
	char *body;              /* Contains body returned */
	char *accept;
	char *accept_encoding;
	char *accept_language;
	char *allow_public;
	char *authorization;
	char *bandwidth;
	char *blocksize;
	char *cache_control;
	char *content_base;
	char *content_encoding;
	char *content_language;
	char *content_location;
	char *content_type;
	char *cookie;
	char *date;
	char *expires;
	char *from;
	char *if_modified_since;
	char *last_modified;
	char *location;
	char *proxy_authenticate;
	char *proxy_require;
	char *range;
	char *referer;
	char *require;
	char *retry_after;
	char *rtp_info;
	char *scale;
	char *server;
	char *session;
	char *speed;
	char *transport;
	char *unsupported;
	char *user_agent;
	char *via;
	char *www_authenticate;
	char *time_shift;
	char *timeshift_status;
	char *time_current;
	char *x_Burst;
	char *x_Retrans;
	//for support rtsp multicast
	unsigned int sdp_connect_ttl;
	char *sdp_connect_addr;
	unsigned int sdp_connect_num_addr;
	unsigned int sdp_media_port;

	WKRtspResponseDecoder* next;
	/* new add for sdp */
	unsigned long rangeStart;
	unsigned long rangeEnd;
	String mStreamType;
	WKRtspResponseDecoder* mHoldSelf;
public:
	bool decode(const char* buf);
	bool decodesdp(const char* buf);
	const char *find_seperator(const char *ptr);
	void dec_dup_warn(char **location, const char *lptr);
	void rtsp_decode_header(const char *lptr);
	unsigned long ascutc2time(const char *ascutc);
	/* code for debug */
	void dump();
	void free_safe(void *ptr);


private:

};
}
#endif //H_WK_RTSP_RESPONSE_DECODER