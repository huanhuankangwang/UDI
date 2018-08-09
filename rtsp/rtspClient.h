#ifndef _H_RTSP_CLIENT_H_
#define _H_RTSP_CLIENT_H_

#include "WKHandler.h"
#include "WKLooper.h"

namespace android
{
	class WKRtspClientHandler : public WKHandler{
		public:
			WKRtspClientHandler();
			~WKRtspClientHandler();
		virtual void dispatchMessage(WKMessage* msg);
	};

	class WKSessionContext;
	class WKStream;
	class WKRtspClient : public WKLooper{
		public:
			WKRtspClient();
			~WKRtspClient();
		private:
			WKSessionContext *mContex;
			WKStream *mStreamCtrl;
			WKRtspClientHandler *mHandler;

		public:
			void doEventLoop();
	};
	
};

#endif//_H_RTSP_CLIENT_H_
