#ifndef H_SK_TSPACKAGE
#define H_SK_TSPACKAGE

#include <threads.h>

namespace android
{

typedef struct RtcpCommonHeader_s
{
#ifdef WORDS_BIGENDIAN
    unsigned short version:2;
    unsigned short padbit:1;
    unsigned short rc:5;
    unsigned short packet_type:8;
#else
    unsigned short rc:5;
    unsigned short padbit:1;
    unsigned short version:2;
    unsigned short packet_type:8;
#endif
    unsigned short length:16;
}RtcpCommonHeader_t;

#define MTU_MAX 2000
typedef struct RtpPackageHeader
{
    #ifdef WORDS_BIGENDIAN
    unsigned int    ph_v: 2;   /* packet type                */
    unsigned int    ph_p: 1;   /* padding flag               */
    unsigned int    ph_x: 1;   /* header extension flag      */
    unsigned int    ph_cc: 4;   /* CSRC count                 */
    unsigned int    ph_m: 1;   /* marker bit                 */
    unsigned int    ph_pt: 7;   /* payload type               */
    unsigned int    ph_seq: 16;   /* sequence number            */
    #else
    unsigned int    ph_cc: 4;   /* CSRC count                 */
    unsigned int    ph_x: 1;   /* header extension flag      */
    unsigned int    ph_p: 1;   /* padding flag               */
    unsigned int    ph_v: 2;   /* packet type                */
    unsigned int    ph_pt: 7;   /* payload type               */
    unsigned int    ph_m: 1;   /* marker bit                 */
    unsigned int    ph_seq: 16;   /* sequence number            */
    #endif
    unsigned long    ph_ts;    /* timestamp                  */
    unsigned long    ph_ssrc;    /* synchronization source     */
    /* The csrc list, header extension and data follow, but can't */
    /* be represented in the struct.                              */
} RtpPackageHeader;

class WKHandler;
class WKTSPackageQueue;
/**
 *
 **/
class WKTSPackage
{
public:
	const static int TS_PACKAGE_SIZE = 188;
	/* 从抓包情况看，一个TCP包通常携带6或7个TS包数据，缓冲池使用20足够 */
	const static int MAX_POOL_SIZE = 1000;
	static WKTSPackage* obain();
	static WKTSPackage* obain(WKHandler* handler);
	/*
	 *TS包一旦被回收将清除消息所携带的数据，回收后外界不应再使用TS包携带的任何数据
	 */
	void recycle();
	bool isInUse();
	void markInUse();
	WKTSPackage();
	virtual ~WKTSPackage();
	unsigned short getPID();
	int testid;
	static int gtestid;
private:
	void clearForRecycle();
public:
	char data[TS_PACKAGE_SIZE];
	WKHandler* handler;
	WKTSPackage* mHoldSelf;
private:
	friend class WKTSPackageQueue;
	static WKTSPackage* sPool;
	static int sPoolSize;

	WKTSPackage* next;
	static Mutex mLock;
	bool mIsInUse;
};
}//:namespace android
#endif //:H_SK_TSPACKAGE

