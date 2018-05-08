#include <utils.h>
#include <time.h>


long uptimeMillis()
{
	time_t ti;
	long mils;
     
    ti = time(NULL); /*获取time_t类型的当前时间*/  

	mils = (long)ti;
	return mils;
}


