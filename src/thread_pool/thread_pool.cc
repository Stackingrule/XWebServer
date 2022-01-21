//#include "thread_pool/thread_pool.hpp"
//
////template< typename T >
//ThreadPool::ThreadPool(int thread_number, int max_requests) :
//        m_thread_number(thread_number), m_max_requests(max_requests),
//        m_stop(false), m_threads(nullptr) {
//
//    if((thread_number <= 0) || (max_requests <= 0) ) {
//        throw std::exception();
//    }
//
//    m_threads = new pthread_t[m_thread_number];
//    if(!m_threads) {
//        throw std::exception();
//    }
//
//    // 创建thread_number 个线程，并将他们设置为脱离线程。
//    for ( int i = 0; i < thread_number; ++i ) {
//        printf( "create the %dth thread\n", i);
//        if(pthread_create(m_threads + i, NULL, Worker, this ) != 0) {
//            delete [] m_threads;
//            throw std::exception();
//        }
//
//        if( pthread_detach( m_threads[i] ) ) {
//            delete [] m_threads;
//            throw std::exception();
//        }
//    }
//}
//
////template< typename T >
//ThreadPool::~ThreadPool() {
//    delete [] m_threads;
//    m_stop = true;
//}
//
////template< typename T >
////bool ThreadPool< T >::Append( T* request )
////{
////    // 操作工作队列时一定要加锁，因为它被所有线程共享。
////    m_queuelocker.Lock();
////    if ( m_workqueue.size() > m_max_requests ) {
////        m_queuelocker.Unlock();
////        return false;
////    }
////    m_workqueue.push_back(request);
////    m_queuelocker.Unlock();
////    m_queuestat.Post();
////    return true;
////}
//
////template< typename T >
//bool ThreadPool::Append( Task&& task )
//{
//    // 操作工作队列时一定要加锁，因为它被所有线程共享。
//    m_queuelocker.Lock();
//    if ( m_workqueue.size() > m_max_requests ) {
//        m_queuelocker.Unlock();
//        return false;
//    }
//    m_workqueue.push_back(std::forward<Task>(task));
//    m_queuelocker.Unlock();
//    m_queuestat.Post();
//    return true;
//}
//
////template< typename T >
//void* ThreadPool::Worker( void* arg ) {
//    ThreadPool* pool = ( ThreadPool* )arg;
//    pool->run();
//    return pool;
//}
//
////template< typename T >
//void ThreadPool::Run() {
//
//    while (!m_stop) {
//        m_queuestat.Wait();
//        m_queuelocker.Lock();
//        if ( m_workqueue.empty() ) {
//            m_queuelocker.Unlock();
//            continue;
//        }
//        auto request = m_workqueue.front();
//        m_workqueue.pop_front();
//        m_queuelocker.Unlock();
//        if ( !request ) {
//            continue;
//        }
//        request->process();
//    }
//
//}
