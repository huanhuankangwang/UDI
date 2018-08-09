#ifndef H_SK_UDP
#define H_SK_UDP
#include <String.h>
#include <pthread.h>
#include "WKHandler.h"
#include "WKLooper.h"
#include "WKTSPackage.h"
#include "WKRtsp.h"
#include "WKRtspCtrl.h"


namespace android
{	
class WKSessionContext;
class WKLooper;
class WKRtsp;
class WKRtspCtrl;

/* class SKCyclerBuffer; */
class WKUDP: public WKHandler
{
public:
	WKUDP() ;
	virtual ~WKUDP();

	bool uniPlay(WKSessionContext* context, unsigned long ip, unsigned short port);
	bool create(WKSessionContext* context, unsigned long ip, unsigned short port);

	bool send(const char* data);
	void stop();
	void stoping();
	bool joinMulticast(int *errornumb, bool fromFcc = false);
	bool leaveMulticast();
	virtual void dispatchMessage(WKMessage* msg);
	const char* getLocalIPString();
	void setRtspCtrl(WKRtspCtrl* rc);
private:
	/* private functions */
	void looper_thread();
	char* get_rtp_or_ts_buffer(char* buffer, int size, int& media_size, int& read_size, RtpPackageHeader **rtpHeader);
	int recvData(char *buf, int size, int timeout, struct sockaddr *from , int *fromlen);
	/* private members */
	pthread_t mThreadId;
	WKLooper* mLooper;
	WKRtspCtrl* mRtspCtrl;
	static bool rmem_max_modified;
	int mSocket;
	bool mIsMulticastIP;
	bool mIsThreadRunning;

	unsigned long mIP;

	String mLocalIPString;
	unsigned short mPort;
	WKSessionContext* mContext;
	unsigned short mNextRTPSeq;
	mutable Mutex mLock;
	/* sp<WKCyclerBuffer> mCyclerBuffer; */
};
}

#endif

