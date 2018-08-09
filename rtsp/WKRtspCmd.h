#ifndef H_WK_RTSP_CTRL_CMD
#define H_WK_RTSP_CTRL_CMD

#include <String.h>

namespace android
{
	class WKSessionContext;
	class WKRtspCmd
	{
	public:
		WKRtspCmd();
		virtual ~WKRtspCmd();
		const char* getDescribeCommand(WKSessionContext* context);
		const char* getSetupCommand(WKSessionContext* context);
		const char* getPlayCommand(WKSessionContext* context);
		const char* getStopCommand(WKSessionContext* context);
		const char* getPauseCommand(WKSessionContext* context);
		const char* getGetParameterCommand(WKSessionContext* context);

		String cmdbuf;
	};
}
#endif	/* H_WK_RTSP_CTRL_CMD */
