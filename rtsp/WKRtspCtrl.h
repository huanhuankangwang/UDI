#ifndef H_SK_RTSP_CTRL
#define H_SK_RTSP_CTRL

#include <threads.h>
#include <String.h>
#include "WKHandler.h"
#include "WKStreamCtrl.h"
#include "WKRtsp.h"

#include "utils.h"

namespace android
{
/* enum WK_STREAM_TYPE */
/* { */
/* 	WK_STREAM_MODE_NULL = 0, */
/* 	WK_STREAM_MODE_LIVE = 1<<0, */
/* 	WK_STREAM_MODE_SHIFT = 1 << 1, */
/* 	WK_STREAM_MODE_VOD = 1 << 2, */
/* }; */
enum WK_RTSP_CTRL_STATE
{
	WK_RTSP_CTRL_STATE_IDLE = 0,
	WK_RTSP_CTRL_STATE_CONNECTING = 0x0001,
	WK_RTSP_CTRL_STATE_ERROR  = 0x0002,
	WK_RTSP_CTRL_STATE_STOPPED = 0x0003,
	WK_RTSP_CTRL_STATE_CONNECTED = 0x0100,
	WK_RTSP_CTRL_STATE_PLAYING = 0x0101,
	WK_RTSP_CTRL_STATE_PAUSEING = 0x0102,
	WK_RTSP_CTRL_STATE_TRICKING = 0x0103,
};

enum WK_RTSP_TRANS_TYPE
{
	WK_RTSP_TRANS_TCP = 0,
	WK_RTSP_TRANS_UDP = 1,
	WK_RTSP_TRANS_RTP_TCP = 2,
	WK_RTSP_TRANS_RTP_UDP = 3,
};

class WKRtsp;
class WKMessage;
class WKLooper;
class WKSessionContext;
class WKRtspRequest;
class WKUDP;
class WKStreamCtrl;

class WKRtspCtrl: public WKHandler
{
public:
	WKRtspCtrl();
	virtual ~WKRtspCtrl();
	void releaseSingle(bool isStop = false);
	bool connect();
	bool describe();
	bool setup();
	bool play(float scale, int mode, const char* range);
	bool doPlay(float scale, const char* range);
	bool stop();
	bool doStop();
	bool pause();
	bool doPause();
	bool getParameter(int burstMode);
	void setContext(WKSessionContext* context);
	void setStream(WKStreamCtrl* stream) { mStream = stream; };
	void setUDPCtrl(WKUDP* uc);
	/* WK_STREAM_TYPE getCurrentStreamType(); */
	virtual void dispatchMessage(WKMessage* msg);
	WK_RTSP_CTRL_STATE getState() { return mState; };
	WKRtsp* getRtsp() {return mRtsp; };
	void setTransProtocal(int type) { mTransProtocol = type; };
	int getTransProtocal() { return mTransProtocol; };
	String getLastPlayRange() { return mLastPlayRange; };
	bool isZteServer() { return mIsZteServ; };
private:
	void appendRequest(WKRtspRequest* r);
	WKRtspRequest* findRequest(int seq);
	void setState(WK_RTSP_CTRL_STATE state);
private:
	WKRtsp* mRtsp;
	WKUDP* mUDPCtrl;
	WKLooper* mLooper;
	WKSessionContext* mContext;
	WKRtspRequest* mRequest;
	WKStreamCtrl* mStream;
	/* WK_STREAM_TYPE mStreamType; */
	int mCseq;
	int mVodEndTime;
	int mPlayMode;
	int mLastScale;
	String mLastPlayRange;
	bool mHearBeatStarted;
	WK_RTSP_CTRL_STATE mState;
	String mSession;
	Mutex mLock;
	Condition mSignal;
	U16 mUDPPort;
	bool mIsStopping;
	int mTransProtocol;
	bool mIsZteServ;
};
}

#endif

