#include "WKRtspRequest.h"


using namespace android;

WKRtspRequest::WKRtspRequest()
{
	mCseq = 0;
	mRequest = UNKOWN;
	next = NULL;
}
WKRtspRequest::~WKRtspRequest()
{
	next = NULL;
}
WKRtspRequest::WKRtspRequest(int seq, WK_RTSP_REQUEST request)
{
	mCseq = seq;
	mRequest = request;
}

WK_RTSP_REQUEST WKRtspRequest::getRequest()
{
	return mRequest;
}
int WKRtspRequest::getCseq()
{
	return mCseq;
}
