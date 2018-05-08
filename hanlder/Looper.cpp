#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>

#include "Looper.h"
#define   ALOGW(...)


using namespace android;

enum LOOPER_STATUS{
	ALOOPER_POLL_WAKE = 0,
	ALOOPER_POLL_TIMEOUT,

	ALOOPER_POLL_ERROR,
};

#define   EPOLL_MAX_EVENTS  16

Looper::Looper() {
    int wakeFds[2];
    int result = pipe(wakeFds);

    mWakeReadPipeFd = wakeFds[0];
    mWakeWritePipeFd = wakeFds[1];

    result = fcntl(mWakeReadPipeFd, F_SETFL, O_NONBLOCK);

    result = fcntl(mWakeWritePipeFd, F_SETFL, O_NONBLOCK);

    // Allocate the epoll instance and register the wake pipe.
    mEpollFd = epoll_create(8);

    struct epoll_event eventItem;
    memset(& eventItem, 0, sizeof(epoll_event)); // zero out unused members of data field union
    eventItem.events = EPOLLIN;
    eventItem.data.fd = mWakeReadPipeFd;
    result = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mWakeReadPipeFd, & eventItem);
}

Looper::~Looper() {
    close(mWakeReadPipeFd);
    close(mWakeWritePipeFd);
    close(mEpollFd);
}

int Looper::pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData){

    // Poll.
    int result = ALOOPER_POLL_WAKE;

    struct epoll_event eventItems[4];
    int eventCount = epoll_wait(mEpollFd, eventItems, EPOLL_MAX_EVENTS, timeoutMillis);

    // Acquire lock.
    //mLock.lock();

    // Check for poll error.
    if (eventCount < 0) {
        if (errno == EINTR) {
            goto Done;
        }
        ALOGW("Poll failed with an unexpected error, errno=%d", errno);
        result = ALOOPER_POLL_ERROR;
        goto Done;
    }

    // Check for poll timeout.
    if (eventCount == 0) {
        result = ALOOPER_POLL_TIMEOUT;
        goto Done;
    }

    for (int i = 0; i < eventCount; i++) {
        int fd = eventItems[i].data.fd;
        uint32_t epollEvents = eventItems[i].events;
        if (fd == mWakeReadPipeFd) {
            if (epollEvents & EPOLLIN) {
                awoken();
            } else {
                ALOGW("Ignoring unexpected epoll events 0x%x on wake read pipe.", epollEvents);
            }
        }
    }
Done:
	// Release lock.
    //mLock.unlock();
   
    return result;
}


void Looper::wake() {

    ssize_t nWrite;
    do {
        nWrite = write(mWakeWritePipeFd, "W", 1);
    } while (nWrite == -1 && errno == EINTR);

    if (nWrite != 1) {
        if (errno != EAGAIN) {
            ALOGW("Could not write wake signal, errno=%d", errno);
        }
    }
}

void Looper::awoken() {
    char buffer[16];
    ssize_t nRead;
    do {
        nRead = read(mWakeReadPipeFd, buffer, sizeof(buffer));
    } while ((nRead == -1 && errno == EINTR) || nRead == sizeof(buffer));
}

