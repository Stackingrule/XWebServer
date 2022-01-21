#ifndef XWEBSERVER_LOCKER_HPP
#define XWEBSERVER_LOCKER_HPP

#include "Noncopyable.hpp"

#include <pthread.h>
#include <semaphore.h>
#include <exception>

/**
 * 线程同步机制封装类
 */
class Locker : public Noncopyable {
public:
    Locker() {
        if(pthread_mutex_init(&m_mutex, NULL) != 0) {
            throw std::exception();
        }
    }

    ~Locker() {
        pthread_mutex_destroy(&m_mutex);
    }

    bool Lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool Unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t * Get() {
        return &m_mutex;
    }
private:
    pthread_mutex_t m_mutex;
};

/**
 * 条件变量类
 */
class Condition : public Noncopyable {
public:
    Condition() {
        if (pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception();
        }
    }
    ~Condition() {
        pthread_cond_destroy(&m_cond);
    }

    bool Wait(pthread_mutex_t* mutex) {
        int ret = 0;
        ret = pthread_cond_wait(&m_cond, mutex);
        return ret == 0;
    }
    bool TimedWait(pthread_mutex_t* mutex, struct timespec t) {
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond, mutex, &t);
        return ret == 0;
    }
    bool Notify() {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool NotifyAll() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
private:
    pthread_cond_t m_cond;
};

/**
 * 信号量类
 */
class Sem : public Noncopyable {
public:
    Sem() {
        if( sem_init( &m_sem, 0, 0 ) != 0 ) {
            throw std::exception();
        }
    }

    Sem( int num ) {
        if( sem_init( &m_sem, 0, num ) != 0 ) {
            throw std::exception();
        }
    }
    ~Sem() {
        sem_destroy( &m_sem );
    }

    /**
     * 等待信号量
     * @return
     */
    bool Wait() {
        return sem_wait( &m_sem ) == 0;
    }
    /**
     * 增加信号量
     * @return
     */
    bool Post() {
        return sem_post( &m_sem ) == 0;
    }

private:
    sem_t m_sem;
};


#endif //XWEBSERVER_LOCKER_HPP