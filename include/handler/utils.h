#ifndef _INCLUDE_UTILS_H_
#define _INCLUDE_UTILS_H_


#define  MODULE_NAME       "MODULE_NAME"

#if 0
//#define  NULL			(0)
#define  LOGI(...)
#define   LOGD(...)
#else
#include <stdio.h>
#define  LOGI(format,args...)	printf("File:[%s] [%s,%d] %s: INFO "format"\r\n",__FILE__,__FUNCTION__,__LINE__,MODULE_NAME,##args)
#define  LOGD(format,args...)	printf("File:[%s] [%s,%d] %s: DEBUG "format"\r\n",__FILE__,__FUNCTION__,__LINE__,MODULE_NAME,##args)

#define  LOGE(format,args...)	printf("File:[%s] [%s,%d] %s: ERROR "format"\r\n",__FILE__,__FUNCTION__,__LINE__,MODULE_NAME,##args)
#endif//

long uptimeMillis();
int sk_mc_ascutc2time(const char *ascutc);


#define  sk_mc_get_processor() NULL
#define  gettid()		NULL



typedef  unsigned short  U16;

#define  milliseconds(mils)		((1000*1000)*mils)

int sk_mc_get_rtsp_protocal();
void sk_mc_set_mul2uni(int val);
int sk_mc_get_need_mul2uni();

int sk_mc_get_mul2uni();

#endif//_INCLUDE_UTILS_H_
