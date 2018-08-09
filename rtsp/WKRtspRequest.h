#ifndef H_WK_RTSP_REQUEST
#define H_WK_RTSP_REQUEST


#include <String.h>


namespace android
{
enum WK_RTSP_REQUEST
{
	UNKOWN = 0,
	DESCRIBE = 1 << 0,
	SETUP = 1 << 1,
	PLAY = 1 << 2,
	PAUSE = 1 << 3,
	GETPARAMETER = 1 << 4,
	STOP = 1 << 5,
	GETPARAMETER_BURST_OPEN = 1 << 6,
	GETPARAMETER_BURST_CLOSE = 1 << 7,
	SETPARAMETER = 1 << 8,
	PACKAGERETRANS = 1 << 9,
	XBURST = 1 << 10,
	OPTIONS = 1 << 11,
};
class WKRtspRequest
{
public:
	WKRtspRequest();
	WKRtspRequest(int seq, WK_RTSP_REQUEST request);
	virtual ~WKRtspRequest();
	WK_RTSP_REQUEST getRequest();
	int getCseq();
public:
	WKRtspRequest* next;
	int mCseq;
	WK_RTSP_REQUEST mRequest;
};
}
#endif //H_WK_RTSP_REQUEST
