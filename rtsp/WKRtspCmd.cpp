#include "WKRtspCmd.h"
#include "WKSessionContext.h"
#include "wk_android_std.h"

using namespace android;


WKRtspCmd::WKRtspCmd()
{
	cmdbuf.clear();
}
WKRtspCmd::~WKRtspCmd()
{
	cmdbuf.clear();

}
const char* WKRtspCmd::getDescribeCommand(WKSessionContext* context)
{
	cmdbuf.clear();
	cmdbuf.appendFormat("DESCRIBE %s RTSP/1.0\r\n", context->mUrl.string());
	cmdbuf.appendFormat("CSeq: %u\r\n", context->mCseq);
	cmdbuf.append("Accept: application/sdp \r\n");
	cmdbuf.append("User-Agent: CTC/2.0\r\n");
	cmdbuf.append("\r\n");
	return cmdbuf.string();
}
const char* WKRtspCmd::getSetupCommand(WKSessionContext* context)
{
	cmdbuf.clear();
	cmdbuf.appendFormat("SETUP %s RTSP/1.0\r\n", context->mUrl.string());
	cmdbuf.appendFormat("CSeq: %u\r\n", context->mCseq);
	cmdbuf.appendFormat("Transport: MP2T/TCP;unicast;destination=10.224.1.252;interleaved=0-1,MP2T/RTP/TCP;unicast;destination=10.224.1.252;interleaved=0-1\r\n");
	cmdbuf.append("User-Agent: CTC/2.0\r\n");
	cmdbuf.append("\r\n");
	return cmdbuf.data();
}
const char* WKRtspCmd::getPlayCommand(WKSessionContext* context)
{
	cmdbuf.clear();
	cmdbuf.appendFormat("PLAY %s RTSP/1.0\r\n", context->mUrl.string());
	cmdbuf.appendFormat("CSeq: %u\r\n", context->mCseq);
	cmdbuf.appendFormat("Session: %s\r\n", context->mSession.string());
	// cmdbuf.append("scale 1.0 \r\n");
	cmdbuf.appendFormat("Scale: %.1f\r\n", context->mScale);
	// LOGD("scale %d %.1f",context->mScale,context->mScale);
	if (strncasecmp(context->mBeginTime.string(), "now", 3) == 0 || strncasecmp(context->mBeginTime.string(), "beginning", 3) == 0)
		cmdbuf.appendFormat("Range: ntp=%s- \r\n","end");//context->mBeginTime.string());
	else
		cmdbuf.appendFormat("Range: clock=%s- \r\n", "end");//context->mBeginTime.string());
	cmdbuf.append("User-Agent: CTC/2.0\r\n");
	cmdbuf.append("\r\n");
	
	return cmdbuf.string();
}
const char* WKRtspCmd::getStopCommand(WKSessionContext* context)
{
	cmdbuf.clear();
	cmdbuf.appendFormat("TEARDOWN %s RTSP/1.0\r\n", context->mUrl.data());
	cmdbuf.appendFormat("CSeq: %u\r\n", context->mCseq);
	cmdbuf.appendFormat("Session: %s\r\n", context->mSession.string());
	cmdbuf.append("User-Agent: CTC/2.0\r\n");
	cmdbuf.append("\r\n");
	return cmdbuf.string();
}
const char* WKRtspCmd::getPauseCommand(WKSessionContext* context)
{
	cmdbuf.clear();
	cmdbuf.appendFormat("PAUSE %s RTSP/1.0\r\n", context->mUrl.string());
	cmdbuf.appendFormat("CSeq: %u\r\n", context->mCseq);
	cmdbuf.appendFormat("Session: %s\r\n", context->mSession.string());
	cmdbuf.append("User-Agent: CTC/2.0\r\n");
	cmdbuf.append("\r\n");
	return cmdbuf.string();
}
const char* WKRtspCmd::getGetParameterCommand(WKSessionContext* context)
{
	cmdbuf.clear();
	cmdbuf.appendFormat("GET_PARAMETER %s RTSP/1.0\r\n", context->mUrl.string());
	cmdbuf.appendFormat("CSeq: %u\r\n", context->mCseq);
	cmdbuf.appendFormat("Session: %s\r\n", context->mSession.string());
	cmdbuf.append("User-Agent: CTC/2.0\r\n");
	cmdbuf.append("x-Timeshift_Range\r\n");
	cmdbuf.append("x-Timeshift_Current\r\n");
	cmdbuf.append("\r\n");
	return cmdbuf.string();
}
