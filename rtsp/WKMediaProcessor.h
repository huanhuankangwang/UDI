#ifndef H_SK_MEDIA_PROCESSOR
#define H_SK_MEDIA_PROCESSOR

#include "WKHandler.h"


#define MAX_SUBTITLE_NUM (32)


//8M bytes size ring buffer
#define	PROCESSOR_RINGBUFFER_LENGTH		0x800000
#define 	UPPER_LIMIT                0x400000
#define 	DOWN_LIMIT                 0x200000


#define 	LOST_PACKET_MAX				100		// ����ܼ�¼�Ķ�������

#define     MAX_CACHE_COUNT  100  		// ���ֻ��������ݿ�����

#define 	MAX_BLOCK_SIZE	1500		// ���ݿ�payload�������ֵ
#define     MAX_BITRATE_COUNTS   10  	//ÿMAX_BITRATE_COUNTS�������������ȡһ��ƽ��ֵ

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

// ��¼����
typedef struct data_lostmap{
	unsigned char* addr[LOST_PACKET_MAX];
	unsigned int seq[LOST_PACKET_MAX];
}data_lostmap_t;
// ���������ݿ�ͷ��
typedef struct block_head{
	unsigned int block_size;
	unsigned char* payload_addr;
	unsigned int payload_size;
	unsigned int seq;
}block_head_t;
// ����ͷ��
typedef struct RingBufferHead
{
	unsigned char *r_addr; // ��ָ��
	unsigned char *w_addr; // дָ��
	unsigned char *buf; // ��������(��������ͷ��)
	unsigned int  buffersize; // ʵ�������������ݵ�buffer��С(����������ͷ��)
	unsigned char *buf_start; // ʵ�ʻ�������buffer��ʼ��ַ
	unsigned char *buf_end; // ʵ�ʻ�������buffer������ַ
	unsigned int  data_count; // ʵ�ʻ��������ݿ����

	int valid_payload_size;//ʵ�ʻ�������ݳ���
	
	unsigned int expect_seq; // Ԥ����һ��seq��id
	int base_seq; // ��ǰbuffer�����д���id
	data_lostmap_t lostMap; // ������¼
	int lostIndex; // ������¼����index
	int lostCount;	// ��������
}RingBufferHead_t;
// ��ȡ����rtp��
typedef struct RtpReadPackage
{
	int seq; // rtp ���к�
	int bufferSize; // ���rtp����buffer ��С
	char * buffer;	// ��ŵ���rtp��
	int dataLength; //ʵ�ʵ���rtp���ݴ�С
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
	bool isFirstPts;	/* �Ƿ���һ����Ŀ���ĵ�һ��pts */
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

