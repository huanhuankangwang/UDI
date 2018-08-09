#include <utils.h>
#include <time.h>

#include <string.h>

enum SK_RTSP_TRANS_TYPE
{
	WK_RTSP_TRANS_TCP = 0,
	WK_RTSP_TRANS_UDP = 1,
	WK_RTSP_TRANS_RTP_TCP = 2,
	WK_RTSP_TRANS_RTP_UDP = 3,
};

static int mRtspProtocal = WK_RTSP_TRANS_TCP;
static int mMul2Uni = 0;
static int mNeedMul2Uni = 0;

int sk_mc_get_need_mul2uni()
{
	return mNeedMul2Uni;
}

long uptimeMillis()
{
	time_t ti;
	long mils;
     
    ti = time(NULL); /*获取time_t类型的当前时间*/  

	mils = (long)ti;
	return mils;
}

int sk_mc_ascutc2time(const char *ascutc)
{
    struct tm t;

	if(NULL == ascutc || strlen(ascutc) < 15)
	{
		return 0;
	}

	t.tm_year = (ascutc[0] - '0') * 1000 + (ascutc[1] - '0') * 100
	            + (ascutc[2] - '0') * 10 + (ascutc[3] - '0') - 1900;
	t.tm_mon = (ascutc[4] - '0') * 10 + (ascutc[5] - '0') - 1 ;
	t.tm_mday = (ascutc[6] - '0') * 10 + (ascutc[7] - '0');
	t.tm_hour = (ascutc[9] - '0') * 10 + (ascutc[10] - '0');
	t.tm_min = (ascutc[11] - '0') * 10 + (ascutc[12] - '0');
	t.tm_sec = (ascutc[13] - '0') * 10 + (ascutc[14] - '0');

	
	//return mktime(&t);
	return timegm(&t);
}

int sk_mc_get_rtsp_protocal()
{
    return mRtspProtocal;
}

void sk_mc_set_mul2uni(int val)
{
	mMul2Uni = val;
}

int sk_mc_get_mul2uni()
{
	return mMul2Uni;
}


