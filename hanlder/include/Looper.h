#ifndef _LOOPER_H_
#define _LOOPER_H_

#define   NULL   0

namespace android
{
class Looper
{
public:
	Looper();
	virtual ~Looper();
public:
	void wake();

	int pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData);
    inline int pollOnce(int timeoutMillis) {
        return pollOnce(timeoutMillis, NULL, NULL, NULL);
    }
	
private:
    int mWakeReadPipeFd;  // immutable
    int mWakeWritePipeFd; // immutable
    int mEpollFd; // immutable

	void awoken();
};
}
#endif//_LOOPER_H_
