#include <utils.h>
#include <time.h>


long uptimeMillis()
{
	time_t ti;
	long mils;
     
    ti = time(NULL); /*��ȡtime_t���͵ĵ�ǰʱ��*/  

	mils = (long)ti;
	return mils;
}


