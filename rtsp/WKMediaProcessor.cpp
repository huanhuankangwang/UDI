#include <String.h>
#include "WKMediaProcessor.h"
#include "WKTSPackage.h"
#include "WKTSPackageQueue.h"
#include "WKMessage.h"
#include "WKMessageQueue.h"
#include "WKLooper.h"
#include "WKSessionContext.h"
#include "wk_android_std.h"


#define MODULE_NAME "SKIPTV_MEDIA_PROCESSOR"
#define REGISTRATION_DESCRIPTOR 5

#define VIDEO_STREAM_AVS           0x42
#define AUDIO_STREAM_MPEG2AAC2  0x11
#define AUDIO_STREAM_AVS        0x43

/* descriptor ids */
#define DVB_SUBT_DESCID             0x59
#define ISO_639_LANGUAGE_DESCRI     0x0a /* ISO 639 language descriptor */

#define STREAM_TYPE_VIDEO_MPEG1         0x01
#define STREAM_TYPE_VIDEO_MPEG2         0x02
#define STREAM_TYPE_AUDIO_MPEG1         0x03
#define STREAM_TYPE_AUDIO_MPEG2         0x04
#define STREAM_TYPE_PRIVATE_SECTION     0x05
#define STREAM_TYPE_PRIVATE_DATA        0x06
#define STREAM_TYPE_AUDIO_MPEG2_AAC     0x0f
#define STREAM_TYPE_VIDEO_MPEG4         0x10
#define STREAM_TYPE_AUDIO_MPEG4_AAC     0x11
#define STREAM_TYPE_VIDEO_H264          0x1b
#define STREAM_TYPE_VIDEO_H265          0x24
#define STREAM_TYPE_VIDEO_AVS           0x42
#define STREAM_TYPE_VIDEO_VC1           0xea
#define STREAM_TYPE_VIDEO_DIRAC         0xd1
#define STREAM_TYPE_VIDEO_AVS2          0xd2
#define STREAM_TYPE_AUDIO_LPCM          0x80
#define STREAM_TYPE_AUDIO_AC3           0x81
#define STREAM_TYPE_AUDIO_HDMV_DTS      0x82
#define STREAM_TYPE_AUDIO_TRUEHD        0x83
#define STREAM_TYPE_AUDIO_DDPlus        0x84
#define STREAM_TYPE_AUDIO_DTS           0x8a
#define STREAM_TYPE_AUDIO_DTS_HDMA      0x86
#define STREAM_TYPE_AUDIO_EAC3          0x87
#define STREAM_TYPE_SUBTITLE_DVB        0x100

#define DMA_BUFFER_SIZE  50*188

#define     RB_HEAD_SIZE   sizeof(RingBufferHead_t)    // 缓存区头部大小
#define 	BLOCK_HEAD_SIZE	 sizeof(block_head_t)	// 数据块头部大小
#define 	RB_ALIGN 		8 	// 内存对齐, 必须为2^n 
#define RB_GET_BLOCK_ADDR(buf) ((buf) + BLOCK_HEAD_SIZE)
#define RB_GET_ALIGN_SIZE(size) ((unsigned int)(size + RB_ALIGN - 1) & ~(RB_ALIGN - 1)) // 队列对齐后大小

using namespace android;
WKMediaProcessor* mProcessorInstance = NULL;

static bool mIsTrick = false;
static bool mIsPause = false;
static bool mbEOF = false;
static bool mLeaveChannelMode = false;
static bool mIsVideoHide = false;
static int mTrickRate = 1;
static long mShowBufferDelay = 3000;

WKMediaProcessor::WKMediaProcessor(WKSessionContext* context)
{


}
WKMediaProcessor::~WKMediaProcessor()
{

	//mQueue = NULL;
}
void WKMediaProcessor::dispatchMessage(WKMessage* msg)
{

}
void WKMediaProcessor::resetPts()
{
    //(mLooper->getMessageQueue())->removeMessages(SK_TS_PACKAGE);
	isFirstPts = true;
    currentTimeSecond = 0;
    //mQueue->removeAll();
}

void WKMediaProcessor::stop(int isChannel)
{

}
void WKMediaProcessor::start()
{
	
}

void WKMediaProcessor::restart()
{
	// not restart thread.
	
}

void WKMediaProcessor::pause()
{
}

void WKMediaProcessor::resume()
{
}

void WKMediaProcessor::seek(double time, bool isHls, bool isRevert)
{	
}


void WKMediaProcessor::trick(int rate, bool isHls)
{
	
	
}

void WKMediaProcessor::stopTrick()
{
	LOGD("set stoptrick");
}

void WKMediaProcessor::setVideoWindow(int x, int y, int w, int h)
{
}

int WKMediaProcessor::videoShow()
{

}

int WKMediaProcessor::videoHide()
{
}

int WKMediaProcessor::getVolume()
{
	return 0;
}

bool WKMediaProcessor::setVolume(int value)
{
//	LOGE("set volume %d", value);
	
}

void WKMediaProcessor::setEventHandler(WKHandler* h)
{
}

int WKMediaProcessor::getAudioChannel()
{

	return 0;
}

bool WKMediaProcessor::setAudioChannel(int n)
{
    LOGD("set audio channel:%d", n);
	return false;
}

void WKMediaProcessor::setAudioTrack()
{


   return;

}

int WKMediaProcessor::getAudioTrack()
{

    return 1;
    //return audioInfo[currAudio].language_name;
}

int WKMediaProcessor::getAudioPID()
{


}

const char * WKMediaProcessor::getAudioLanguage(int pid)
{

    return NULL;
}

const int * WKMediaProcessor::getAudioPIDs(int &totalNum)
{

    return NULL;
}

void WKMediaProcessor::setAudioPID(int pid)
{
    LOGD("setAudioPID pid : %d \n", pid);


}

void WKMediaProcessor::setDiscardPackage(bool discard)
{
	LOGE("discardPackage :%d", (int)discard);

}

bool WKMediaProcessor::handle_packet(const unsigned char *packet, bool isPATorPMT) // ture pat,false pmt
{
	return true;
}


void  WKMediaProcessor::computeRate()
{


}

void WKMediaProcessor::addAudioParam(int pid, int format)
{

}


unsigned long long WKMediaProcessor::handle_pcr(const unsigned char *section)
{

    return 0;
}

void WKMediaProcessor::handle_pat(const unsigned char *section, int section_len)
{


}

void WKMediaProcessor::addAudioInfo(int pid, const char *language_name,int index)
{

}

void WKMediaProcessor::handle_pmt(const unsigned char *section, int section_len)
{


	// psi_info->prog_num = audio_num;
}
bool WKMediaProcessor::parse_section_header(SKSectionHeader *h, const unsigned char **pp, const unsigned char *p_end)
{


	return true;
}

int WKMediaProcessor::mergePMT(char* packet, char *PMTbuffer, bool& isMergeStart, bool& isMergeOver)
{
    int ret = 0;

    return ret;
}

int WKMediaProcessor::get8(const unsigned char **pp, const unsigned char *p_end)
{

	int c;

	return c;
}
int WKMediaProcessor::get16(const unsigned char **pp, const unsigned char *p_end)
{
	int c;

	return c;
}

bool WKMediaProcessor::resetSurface()
{
	bool ret = false;

    return ret;
}

int WKMediaProcessor::bufferAvailable()
{
    int ret = 0;


    return ret;
}


/*不支持RTP包排序功能(不打开宏SK_SUPPORT_RTP_SORT)时，参数seq没用*/
int WKMediaProcessor::InputDataMediaProcessor(char* wr_buf, int rd_len, unsigned int seq)
{
	return -1;
}

int WKMediaProcessor::processpackage(char* buffer, bool onlyCheckPSI)
{

	return 0;
}

int WKMediaProcessor::process(char* buffer, int len, bool onlyCheckPSI, bool onlyInjectSubtitle)
{
	int bufferLen = 0;


	return 0;
}

void WKMediaProcessor::setEOF()
{
    LOGE("set EOF\n");

    mbEOF = true;
}

void WKMediaProcessor::setRefreshSeekTime(int seektime)
{
	LOGE("set setRefreshSeekTime, seektime:%d\n", seektime);

}

bool WKMediaProcessor::isRunning()
{
    return 0;
}


void WKMediaProcessor::looper_thread()
{
	
}

void WKMediaProcessor::handle_findmediastream(const unsigned char *packet)
{

}

void WKMediaProcessor::start_stream_Player()
{

}
 
