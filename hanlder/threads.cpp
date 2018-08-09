#include <threads.h>

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>


using namespace android;

Mutex::Mutex() {
    pthread_mutex_init(&mMutex, NULL);
}

Mutex::Mutex(const char* name) {
    pthread_mutex_init(&mMutex, NULL);
}

Mutex::Mutex(int type, const char* name) {
    if (type == SHARED) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    } else {
        pthread_mutex_init(&mMutex, NULL);
    }
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&mMutex);
}
int Mutex::lock() {
    return -pthread_mutex_lock(&mMutex);
}
void Mutex::unlock() {
    pthread_mutex_unlock(&mMutex);
}

int Mutex::tryLock() {
    return -pthread_mutex_trylock(&mMutex);
}


Condition::Condition() {
    pthread_cond_init(&mCond, NULL);
}
Condition::Condition(int type) {
    if (type == SHARED) {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&mCond, &attr);
        pthread_condattr_destroy(&attr);
    } else {
        pthread_cond_init(&mCond, NULL);
    }
}
Condition::~Condition() {
    pthread_cond_destroy(&mCond);
}
int Condition::wait(Mutex& mutex) {
    return -pthread_cond_wait(&mCond, &mutex.mMutex);
}
int Condition::waitRelative(Mutex& mutex, nsecs_t reltime) {
#if defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE)
    struct timespec ts;
    ts.tv_sec  = reltime/1000000000;
    ts.tv_nsec = reltime%1000000000;
    return -pthread_cond_timedwait_relative_np(&mCond, &mutex.mMutex, &ts);
#else // HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
    struct timespec ts;
#if defined(HAVE_POSIX_CLOCKS)
    clock_gettime(CLOCK_REALTIME, &ts);
#else // HAVE_POSIX_CLOCKS
    // we don't support the clocks here.
    struct timeval t;
    gettimeofday(&t, NULL);
    ts.tv_sec = t.tv_sec;
    ts.tv_nsec= t.tv_usec*1000;
#endif // HAVE_POSIX_CLOCKS
    ts.tv_sec += reltime/1000000000;
    ts.tv_nsec+= reltime%1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec  += 1;
    }
    return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);
#endif // HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
}
void Condition::signal() {
    pthread_cond_signal(&mCond);
}
void Condition::broadcast() {
    pthread_cond_broadcast(&mCond);
}


void Condition::signal(WakeUpType type) {
	if (type == WAKE_UP_ONE) {
		signal();
	} else {
		broadcast();
	}
}

