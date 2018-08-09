#ifndef H_SK_STREAM_CTRL
#define H_SK_STREAM_CTRL

#include <WKStream.h>

namespace android
{
class WKMessage;
class WKLooper;
class WKRtspCtrl;
//class WKSessionContext;
//class WKURLParser;
class WKStream;
class WKUDP;
class WKStreamCtrl: public WKStream
{
public:
	WKStreamCtrl(WKSessionContext* context);
	virtual ~WKStreamCtrl();
	virtual void dispatchMessage(WKMessage* msg);
	bool connect(const char *url);
	bool play();
	bool stop();
	void stoping();
	bool pause();
	bool trick(int scale);
	bool mul2uni();
	bool seekTo(int mode, const char* t);
	void releaseRtspSingle(bool isStop=false);
/*
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
*/
private:
	WKRtspCtrl* mRtspCtrl;
	WKUDP* mUDPCtrl;
	//WKSessionContext* mContext;
	WKLooper* mLooper;
	//WKURLParser* mUrlParser;
};
}
#endif //H_SK_STREAM_CTRL
