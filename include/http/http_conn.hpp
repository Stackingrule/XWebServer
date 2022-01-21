#ifndef XWEBSERVER_HTTP_CONN_HPP
#define XWEBSERVER_HTTP_CONN_HPP

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
#include <atomic>

#include "buffer.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "log.hpp"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    void Init(int sockFd, const sockaddr_in& addr);

    ssize_t Read(int* saveErrno);

    ssize_t Write(int* saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;

    sockaddr_in GetAddr() const;

    bool Process();

    int ToWriteBytes() {
        return iov[0].iov_len + iov[1].iov_len;
    }

    bool IsKeepAlive() const {
        return request.IsKeepAlive();
    }
public:
    static bool isET;
    static const char* srcDir;
    static std::atomic<int> m_user_count;   // 连接的用户数
private:

    int m_sockFd;           // 该HTTP连接的socket
    sockaddr_in m_address;  // 该HTTP连接的对方的socket地址

    bool m_isClose;

    int iovCnt;
    struct iovec iov[2];

    Buffer readBuff; // 读缓冲区
    Buffer writeBuff; // 写缓冲区

    HttpRequest request;
    HttpResponse response;
};

#endif //XWEBSERVER_HTTP_CONN_HPP
