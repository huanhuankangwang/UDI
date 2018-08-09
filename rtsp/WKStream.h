#ifndef _H_WK_STREAM_H_
#define _H_WK_STREAM_H_

#include "WKHandler.h"

namespace android
{

class WKSessionContext;
class WKURLParser;

class WKStream: public WKHandler
{
public:
	WKStream(WKSessionContext* context);
	~WKStream();
	virtual void dispatchMessage(WKMessage* msg);
	virtual bool connect(const char *url) = 0;
	virtual bool play() = 0;
	virtual bool stop() = 0;
	virtual void stoping() = 0;
	virtual bool pause() = 0;
	virtual bool trick(int scale) = 0;
	virtual bool mul2uni() = 0;
	virtual bool seekTo(int mode, const char* t) = 0;
	virtual void releaseRtspSingle(bool isStop=false) = 0;

	/*本类中实现*/
	void setTsHandler(WKHandler* h);
	void setEventHandler(WKHandler* h);
	bool isChannel();
	bool isLive();
	bool isMultiLive();
	void ctlBufferState(int burstMode);
	int getTimeshiftLength();
	bool trickAble();
	// 0 - multicase
	// 1 - tcp
	// 2 - tcp/rtp
	// 3 - udp
	// 4 - udp/rtp
	int getCurrentProtocolType();
public:
	WKSessionContext* mContext;
	WKURLParser* mUrlParser;
};
}
#endif//_H_WK_STREAM_H_