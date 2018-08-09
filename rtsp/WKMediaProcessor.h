#ifndef H_SK_MEDIA_PROCESSOR
#define H_SK_MEDIA_PROCESSOR

#include "WKHandler.h"


#define MAX_SUBTITLE_NUM (32)


//8M bytes size ring buffer
#define	PROCESSOR_RINGBUFFER_LENGTH		0x800000
#define 	UPPER_LIMIT                0x400000
#define 	DOWN_LIMIT                 0x200000


#define 	LOST_PACKET_MAX				100		// 最大能记录的丢包个数

#define     MAX_CACHE_COUNT  100  		// 保持缓存区数据块数量

#define 	MAX_BLOCK_SIZE	1500		// 数据块payload区间最大值
#define     MAX_BITRATE_COUNTS   10  	//每MAX_BITRATE_COUNTS个码率算出来后取一次平均值

#define		BUFFER_LENGTH			   MAX_BLOCK_SIZE * MAX_CACHE_COUNT


#define MAX_AUDIOTRACK_NUM (8)

namespace android
{
enum SKMediaProcessorState{
	SK_MEDIA_PROCESSOR_IDLE = 0,
	SK_MEDIA_PROCESSOR_STARTING,
	SK_MEDIA_PROCESSOR_STARTED,
	SK_MEDIA_PROCESSOR_STOPING,
	SK_MEDIA_PROCESSOR_STOPED,
	SK_MEDIA_PROCESSOR_TRICKING,
	SK_MEDIA_PROCESSOR_RESTART
};
typedef struct SKSectionHeader
{

	unsigned char tid;
	unsigned short section_length;
	unsigned short id;
	unsigned char version;
	unsigned char sec_num;
	unsigned char last_sec_num;
}SKSectionHeader;

// 记录丢包
typedef struct data_lostmap{
	unsigned char* addr[LOST_PACKET_MAX];
	unsigned int seq[LOST_PACKET_MAX];
}data_lostmap_t;
// 缓存中数据块头部
typedef struct block_head{
	unsigned int block_size;
	unsigned char* payload_addr;
	unsigned int payload_size;
	unsigned int seq;
}block_head_t;
// 缓存头部
typedef struct RingBufferHead
{
	unsigned char *r_addr; // 读指针
	unsigned char *w_addr; // 写指针
	unsigned char *buf; // 整个缓存(包括缓存头部)
	unsigned int  buffersize; // 实际用来缓存数据的buffer大小(不包括缓存头部)
	unsigned char *buf_start; // 实际缓存数据buffer起始地址
	unsigned char *buf_end; // 实际缓存数据buffer结束地址
	unsigned int  data_count; // 实际缓存中数据块个数

	int valid_payload_size;//实际缓存的数据长度
	
	unsigned int expect_seq; // 预期下一个seq的id
	int base_seq; // 当前buffer中最后写入的id
	data_lostmap_t lostMap; // 丢包记录
	int lostIndex; // 丢包记录数据index
	int lostCount;	// 丢包个数
}RingBufferHead_t;
// 读取单个rtp包
typedef struct RtpReadPackage
{
	int seq; // rtp 序列号
	int bufferSize; // 存放rtp包的buffer 大小
	char * buffer;	// 存放单个rtp包
	int dataLength; //实际单个rtp数据大小
}RtpReadPackage_t;



class WKTSPackage;
//class WKTSPackageQueue;
class WKMessage;
class WKLooper;
class WKSessionContext;

class WKMediaProcessor: public WKHandler
{
public:
	WKMediaProcessor(WKSessionContext* context);
	virtual ~WKMediaProcessor();
	virtual void dispatchMessage(WKMessage* msg);
	//void writePackage(WKTSPackage* pkg);
	void start();
	void restart();
	void stop(int isChannel=0);
	void resetPts();
	void pause();
	void resume();
	void seek(double time=0, bool isHls=false, bool isRevertPlaySeek=false);
	void trick(int rate, bool isHls=false);
	void stopTrick();
	void setVideoWindow(int x, int y, int w, int h);
	int videoShow();
	int videoHide();
	int getVolume();
	bool setVolume(int value);
	void setEventHandler(WKHandler* h);
	int getAudioChannel();
	bool setAudioChannel(int n);
	void setAudioTrack();
	int getAudioTrack();
	int getAudioPID();
	void setAudioPID(int pid);
	const int *getAudioPIDs(int &totalNum);
	const char *getAudioLanguage(int pid);
	bool resetSurface();
	int InputDataMediaProcessor(char* wr_buf, int rd_len, unsigned int seq);
	void setEOF();
	void setRefreshSeekTime(int seektime);
	void setDiscardPackage(bool discard);
	int bufferAvailable();	
	WKHandler* getEventHandler(){ return eventHandler; }
	bool isRunning();
	int getCurrentTime(){ return currentTimeSecond; }

private:
	int mergePMT(char* buffer, char *PMTbuffer,bool& isMergeStart,bool& isMergeOver);
	void handle_findmediastream(const unsigned char *packet); 
	bool handle_packet(const unsigned char *packet, bool isPATorPMT); // ture pat,false pmt
	void handle_pat(const unsigned char *section, int section_len);
	unsigned long long handle_pcr(const unsigned char *section);
	void handle_pmt(const unsigned char *section, int section_len);
	void computeRate();
	int get8(const unsigned char **pp, const unsigned char *p_end);
	int get16(const unsigned char **pp, const unsigned char *p_end);
	bool parse_section_header(SKSectionHeader *h, const unsigned char **pp, const unsigned char *p_end);
	void looper_thread();
	
	int process(char* buffer, int len, bool onlyCheckPSI, bool onlyInjectSubtitle=false);
	int processpackage(char* buffer, bool onlyCheckPSI);
	void start_stream_Player();
	void addAudioParam(int pid, int format);
	void addAudioInfo(int pid, const char *languageCode,int index);
private:
	//CTC_MediaProcessor* mMediaProcessor;
	WKSessionContext* mContext;
	//WKTSPackageQueue* mQueue;
	//VIDEO_PARA_T videoParam;
	//AUDIO_PARA_T audioParam[MAX_AUDIOTRACK_NUM];
	int audio_num;
	int currAudio;
	char pmtBuffer[512];
	bool isPMTMergeOver;
	bool isPMTMergeStart;
	int pmtMergeLength;
	int pmtSectionLength;
	unsigned short pmtpid;
	unsigned short pcrpid;
	SKMediaProcessorState mState;
	bool isFirstPts;	/* 是否是一个节目流的第一个pts */
	bool decodepts;
	bool mStarted;
	signed long long firstPts;
	int currentTimeSecond;
	WKHandler* eventHandler;
	WKLooper* mLooper;
	static int bufferPkg;
	bool mIsThreadRunning;
	int mSeekTime;
	int mbFoundPMT;
	char *mpTSBuffer;
	long packageCount;
	double bitRates[MAX_BITRATE_COUNTS];
	int rateCounts;
	unsigned long long firstpcr;
	unsigned long long pcr1;
	unsigned long long pcr2;
	//AUDIO_INFO audioInfo[MAX_AUDIOTRACK_NUM];
	//SUBT_INFO subtitleInfo[MAX_SUBTITLE_NUM];
	int subtitle_num;
	int currSubtitle;
	//bool countRate;
	/* mutable Mutex mLock; */
	/* Condition mSignal; */
};
}
#endif	/* H_WK_MEDIA_PROCESSOR */

