#ifndef XWEBSERVER_THREAD_POOL_HPP
#define XWEBSERVER_THREAD_POOL_HPP

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include <queue>
#include <functional>

#include "locker/locker.hpp"

// 线程池类，将它定义为模板类是为了代码复用，模板参数T是任务类
//template<typename T>

//class ThreadPool {
//public:
//    using Task = std::function<void()>;
//
//    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
//    ThreadPool(int thread_number = 8, int max_requests = 10000);
//    ~ThreadPool();
//
//    //bool Append(T* request);
//    bool Append(Task && task);
//
//private:
//    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
//    static void* Worker(void* arg);
//    void Run();
//
//private:
//    // 线程的数量
//    int m_thread_number;
//
//    // 描述线程池的数组，大小为m_thread_number
//    pthread_t * m_threads;
//
//    // 请求队列中最多允许的、等待处理的请求的数量
//    int m_max_requests;
//
//    // 请求队列
////    std::list< T* > m_workqueue;
//    std::list< std::function<void()> > m_workqueue;
//
//    // 保护请求队列的互斥锁
//    Locker m_queuelocker;
//
//    // 是否有任务需要处理
//    Sem m_queuestat;
//
//    // 是否结束线程
//    bool m_stop;
//};

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): m_pool(std::make_shared<Pool>()) {
        assert(threadCount > 0);
        for(size_t i = 0; i < threadCount; i++) {
            std::thread([pool = m_pool] {
                std::unique_lock<std::mutex> locker(pool->mtx);
                while(true) {
                    if(!pool->tasks.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if(pool->isClosed) break;
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool() {
        if(static_cast<bool>(m_pool)) {
            {
                std::lock_guard<std::mutex> locker(m_pool->mtx);
                m_pool->isClosed = true;
            }
            m_pool->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(m_pool->mtx);
            m_pool->tasks.emplace(std::forward<F>(task));
        }
        m_pool->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> m_pool;
};


#endif //XWEBSERVER_THREAD_POOL_HPP
