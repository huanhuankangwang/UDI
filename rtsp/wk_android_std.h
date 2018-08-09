#ifndef _SK_DEBUG_H_
#define _SK_DEBUG_H_
#include <stdio.h>
#include "wk_android_message.h"
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__)
#endif
#ifndef LOGE
#define LOGE(...) printf(__VA_ARGS__)
#endif
#ifndef LOGI
#define LOGI(...) printf(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif


