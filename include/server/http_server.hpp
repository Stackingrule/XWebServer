#ifndef XWEBSERVER_HTTP_SERVER_HPP
#define XWEBSERVER_HTTP_SERVER_HPP

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>
#include <functional>

#include "http_conn.hpp"
#include "epoller.hpp"
#include "thread_pool.hpp"
#include "timer.hpp"
#include "log.hpp"

class HttpServer {

public:

    HttpServer(
            int port, int trigMode, int timeoutMS,
            bool OptLinger,
            int threadNum,
            bool openLog, int logLevel, int logQueSize);

    ~HttpServer();

    void Start();

    //void AddSignal(int sig, void( handler )(int));

private:
    bool InitSocket();
    void InitEventMode(int trigMode);
    void AddClient(int fd, sockaddr_in addr);
    void CloseConn(HttpConn* client);

    void DealListen();
    void DealWrite(HttpConn* client);
    void DealRead(HttpConn* client);

    void SendError(int fd, const char*info);
    void ExtentTime(HttpConn* client);  // todo

    void OnRead(HttpConn* client);
    void OnWrite(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;

    static int SetFdNonblock(int fd);

private:
    int m_port;
    bool openLinger;
    //int timeoutMS;  /* 毫秒MS */
    int timeoutMS_;  /* 毫秒MS */
    bool isClose;
    int listenFd;
    char* m_srcDir;

    uint32_t listenEvent;
    uint32_t connEvent;

    std::unique_ptr<ThreadPool> threadpool;
    std::unique_ptr<Epoller> m_epoller;
    std::unordered_map<int, HttpConn> m_users;

    std::unique_ptr<HeapTimer> m_timer;

};

#endif //XWEBSERVER_HTTP_SERVER_HPP
