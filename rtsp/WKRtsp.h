#ifndef H_SK_RTSP
#define H_SK_RTSP

#include <pthread.h>
#include "WKHandler.h"


/*
	WKRtsp 主要是用于开发app
	用于Rtsp相关协议的开发

	rtsp支持四种协议
	
*/
namespace android
{
class WKSessionContext;
class WKRtsp: public WKHandler
{
public:
	typedef enum player_error_e
	{

		MEDIA_ERROR_BASE,
		MEDIA_ERROR_NO_MEMORY, /* 内存不够 */
		MEDIA_ERROR_SYSTEM_FATAL,        /* 操作系统错误 */
		MEDIA_ERROR_INVALID_URL,   /* 不支持的URL */
		MEDIA_ERROR_UNKNOWN_PROFILE,    /* profile格式不正确 */
		MEDIA_ERROR_FILE_NOTEXIT,   /* 文件不存在 */
		MEDIA_ERROR_UNKNOWN_FORMAT,    /* 未知的流格式 */
		MEDIA_ERROR_CREATESOCKET_FAILED,    /* 创建SOCKET失败 */
		MEDIA_ERROR_CONNECTSERVER_FAILED,       /* 连接服务器失败 */
		MEDIA_ERROR_DECODER_EXCEPTION,        /* decoder异常 */
		MEDIA_ERROR_SESSION_EXCEPTION,       /* session 异常 */
		MEDIA_ERROR_SERVER_UNRESPONSE,     /* 服务器未响应 */
		MEDIA_ERROR_SERVER_ERROR,       /* 服务器5xxx错误 */
		MEDIA_ERROR_UNSUPPORT_SERVER,        /* 不支持的视频服务器 */
		MEDIA_ERROR_DISORDER_STATE,   /* 播放状态混乱 */
		MEDIA_ERROR_UNSUPPORT_PROTOCOL,   /* 不支持的协议 */
		MEDIA_ERROR_DECRYPT_FAILED,    /* 解密错误 */
		MEDIA_ERROR_REACH_MAX_LICENSE,       /* 到达最大用户数 */
		MEDIA_ERROR_NO_PERMISSION,
		MEDIA_ERROR_FORCE_EXIT,       /* Verimatrix DRM错误 */
		MEDIA_ERROR_DRM_MICROSOFT,           /* Microsoft DRM错误 */
	} player_error_t;
	static const int MAX_BACKUP_SERVER = 4;
	static const int MAX_RRS_FAILED = 64;
	WKRtsp() ;
	virtual ~WKRtsp();
	bool rtsp_create_client(WKSessionContext *context, int *er);

	//发送命令
	bool sendCommand(const char* cmd);
	void stop();
	virtual void dispatchMessage(WKMessage* msg);
private:
	//解析rtsp url串
	bool rtsp_dissect_url(const char *url);
	int rtsp_create_socket();
	unsigned long *rtsp_failedrrs_query(unsigned long ip, int flag);
	unsigned long *rtsp_failedrrs_add(unsigned long ip, int flag);

	int rtsp_recv_data(char *buf, int size, int timeout);
	bool processTsPackage(const char* prefix);
	bool processResponse(const char* prefix);
public:
	WKSessionContext* mContext;
	char *mUrl;

private:
	unsigned long m_rrs_failed[MAX_RRS_FAILED];
	unsigned long m_rrs_skiped[MAX_RRS_FAILED];

	/* char *mUrl; */
	char *mServerName;
	unsigned short mPort;
	char mRtspToken[8];

	//backup服务器
	int mBackupNum;
	char *mBackupServers[MAX_BACKUP_SERVER];
	unsigned short mBackupPorts[MAX_BACKUP_SERVER];
	bool mIsThreadRunning;

	/*开启的Rtsp服务线程*/
	void looper_thread();
	pthread_t mThreadId;
	WKLooper* mLooper;
};
}

#endif

