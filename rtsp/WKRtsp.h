#ifndef H_SK_RTSP
#define H_SK_RTSP

#include <pthread.h>
#include "WKHandler.h"


/*
	WKRtsp ��Ҫ�����ڿ���app
	����Rtsp���Э��Ŀ���

	rtsp֧������Э��
	
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
		MEDIA_ERROR_NO_MEMORY, /* �ڴ治�� */
		MEDIA_ERROR_SYSTEM_FATAL,        /* ����ϵͳ���� */
		MEDIA_ERROR_INVALID_URL,   /* ��֧�ֵ�URL */
		MEDIA_ERROR_UNKNOWN_PROFILE,    /* profile��ʽ����ȷ */
		MEDIA_ERROR_FILE_NOTEXIT,   /* �ļ������� */
		MEDIA_ERROR_UNKNOWN_FORMAT,    /* δ֪������ʽ */
		MEDIA_ERROR_CREATESOCKET_FAILED,    /* ����SOCKETʧ�� */
		MEDIA_ERROR_CONNECTSERVER_FAILED,       /* ���ӷ�����ʧ�� */
		MEDIA_ERROR_DECODER_EXCEPTION,        /* decoder�쳣 */
		MEDIA_ERROR_SESSION_EXCEPTION,       /* session �쳣 */
		MEDIA_ERROR_SERVER_UNRESPONSE,     /* ������δ��Ӧ */
		MEDIA_ERROR_SERVER_ERROR,       /* ������5xxx���� */
		MEDIA_ERROR_UNSUPPORT_SERVER,        /* ��֧�ֵ���Ƶ������ */
		MEDIA_ERROR_DISORDER_STATE,   /* ����״̬���� */
		MEDIA_ERROR_UNSUPPORT_PROTOCOL,   /* ��֧�ֵ�Э�� */
		MEDIA_ERROR_DECRYPT_FAILED,    /* ���ܴ��� */
		MEDIA_ERROR_REACH_MAX_LICENSE,       /* ��������û��� */
		MEDIA_ERROR_NO_PERMISSION,
		MEDIA_ERROR_FORCE_EXIT,       /* Verimatrix DRM���� */
		MEDIA_ERROR_DRM_MICROSOFT,           /* Microsoft DRM���� */
	} player_error_t;
	static const int MAX_BACKUP_SERVER = 4;
	static const int MAX_RRS_FAILED = 64;
	WKRtsp() ;
	virtual ~WKRtsp();
	bool rtsp_create_client(WKSessionContext *context, int *er);

	//��������
	bool sendCommand(const char* cmd);
	void stop();
	virtual void dispatchMessage(WKMessage* msg);
private:
	//����rtsp url��
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

	//backup������
	int mBackupNum;
	char *mBackupServers[MAX_BACKUP_SERVER];
	unsigned short mBackupPorts[MAX_BACKUP_SERVER];
	bool mIsThreadRunning;

	/*������Rtsp�����߳�*/
	void looper_thread();
	pthread_t mThreadId;
	WKLooper* mLooper;
};
}

#endif

