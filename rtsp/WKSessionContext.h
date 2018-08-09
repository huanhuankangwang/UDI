#ifndef H_SK_SESSION_CONTEXT
#define H_SK_SESSION_CONTEXT

#include <threads.h>
#include <String.h>



namespace android
{
enum THREAD_STATES {
	WK_THREAD_STATE_IDLE        = 0,
	WK_THREAD_STATE_RUNNING               = 1 << 0,
	WK_THREAD_STATE_EXITING        = 1 << 1,
	WK_THREAD_STATE_EXITED          = 1 << 2,
};
/*|0x|multicast/unmulticast|live/shift|udp/tcp|channel/vod */
/* enum STREAM_TYPE{ */
#define WK_STREAM_IDLE 0
#define WK_STREAM_MULTICAST 0x1000
#define WK_STREAM_LIVE 0x0100
#define	WK_STREAM_UDP  0x0010
#define	WK_STREAM_CHANNEL  0x0001
#define	WK_STREAM_LIVE_MULTICAST (WK_STREAM_MULTICAST | WK_STREAM_LIVE | WK_STREAM_UDP | WK_STREAM_CHANNEL)
#define WK_STREAM_LIVE_UNMULTICAST_UDP (WK_STREAM_LIVE | WK_STREAM_UDP | WK_STREAM_CHANNEL)
#define	WK_STREAM_LIVE_UNMULTICAST_TCP (WK_STREAM_LIVE |WK_STREAM_CHANNEL)
#define	WK_STREAM_SHIFT_UNMULTICAST_UDP (WK_STREAM_UDP | WK_STREAM_CHANNEL)
#define	WK_STREAM_SHIFT_UNMULTICAST_TCP WK_STREAM_CHANNEL
/* }; */

enum XBURST_STATE {
	XBURST_STATE_CLOSE = 0,
	XBURST_STATE_OPEN = 1,
	XBURST_STATE_PAUSE = 2,
	XBURST_STATE_RESUME = 3,
};

enum STREAM_PROTOCAL_TYPE {
	STM_PROTO_UNKNOWN = -1,
	STM_PROTO_MULTICAST = 0,
	STM_PROTO_UNICAST_TCP = 1,
	STM_PROTO_UNICAST_TCP_RTP = 2,
	STM_PROTO_UNICAST_UDP = 3,
	STM_PROTO_UNICAST_UDP_RTP = 4,
};

#define MAX_RECV_TIMEOUT (60 * 1000)
#define MAX_MULTIFAILED 3

#define IS_MULTICAST_IP(ip) ( 0xE0<=(ip&0xFF) && (ip&0xFF)<0xF0 )

class WKHandler;
class WKTSPackage;
class WKTSPackageQueue;
class WKRtspResponseDecoder;
class WKSessionContext
{
public:
	static const int MAX_BACKUP_SERVER = 4;
	/* functions */
	WKSessionContext();
	virtual ~WKSessionContext();
	//sp<WKTSPackage> obainTsPkg();
	//void enqueueTsPkg(sp<WKTSPackage> pkg);
	void setTsHandler(WKHandler* h);
	void setRtspHandler(WKHandler* h);
	void setEventHandler(WKHandler* h);
	//void appendResponse(WKRtspResponseDecoder* r);
	THREAD_STATES getReceiveThreadState();
	void setReceiveThreadState(THREAD_STATES state);
	THREAD_STATES getUDPThreadState();
	void setUDPThreadState(THREAD_STATES state);
	unsigned int getStreamType();
	void setStreamType(unsigned int t);
	/* 不应使用这个方法，临时版本先加上 */
	void sessionStop();
	// 如果找不到将返回NULL
	WKRtspResponseDecoder* findResponse(int seq);
	const char* getIGMPUrl();
	void setIGMPUrl(const char* url);
	void setHasRtp(bool has);
	bool getHasRtp();
	short int getServerSinfamily(){return mServerSinfamily;}
	void setServerSinfamily(short int sin_family);	
	unsigned long getLocalIP();
	unsigned long getLocalIP(char* ifname, int size);
	/* members */
	String mUrl;
	int mServerSocket;
	WKSessionContext* mHoldSelf;
	WKHandler* mTsHandler;
	WKHandler* mRtspHandler;
	WKHandler* mEventHandler;
	/* rtsp session members */
	THREAD_STATES mRtspThreadState;
	unsigned int mMultifailCount;
	bool mMul2UniIsFail;
	bool mHasMul2Uni;

	int mCseq;
	String mSession;
	float mScale;
	String mBeginTime;
	
private:
	//sp<SKTSPackageQueue> mQueue;
	mutable Mutex mLock;
	/* String8 mIgmpUrl; */
	/* THREAD_STATES mRtspThreadState; */
	THREAD_STATES mUDPThreadState;
	/* STREAM_TYPE mStreamType; */
	unsigned int mStreamType;
	bool mHasRtp;
	short int mServerSinfamily;

};
}

#endif	// H_SK_SESSION_CONTEXT
