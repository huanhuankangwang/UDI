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


#endif//_INCLUDE_UTILS_H_
