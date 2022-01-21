#include "http_conn.hpp"

const char* HttpConn::srcDir;
std::atomic<int> HttpConn::m_user_count;
bool HttpConn::isET;

HttpConn::HttpConn() {
    m_sockFd = -1;
    m_address = { 0 };
    m_isClose = true;
}

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::Init(int sockFd, const sockaddr_in &addr) {
    assert(sockFd > 0);
    m_user_count++;
    m_address = addr;
    m_sockFd = sockFd;
    writeBuff.RetrieveAll();
    readBuff.RetrieveAll();
    m_isClose = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", m_sockFd, GetIP(), GetPort(), (int)m_user_count);

}

ssize_t HttpConn::Read(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff.ReadFd(m_sockFd, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}

ssize_t HttpConn::Write(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(m_sockFd, iov, iovCnt);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov[0].iov_len + iov[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov[0].iov_len) {
            iov[1].iov_base = (uint8_t*) iov[1].iov_base + (len - iov[0].iov_len);
            iov[1].iov_len -= (len - iov[0].iov_len);
            if(iov[0].iov_len) {
                writeBuff.RetrieveAll();
                iov[0].iov_len = 0;
            }
        }
        else {
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len;
            iov[0].iov_len -= len;
            writeBuff.Retrieve(len);
        }
    } while(isET || ToWriteBytes() > 10240);
    return len;
}


bool HttpConn::Process() {
    request.Init();
    if(readBuff.ReadableBytes() <= 0) {
        return false;
    }
    else if(request.ParseRequest(readBuff)) {
        LOG_DEBUG("%s", request.GetPath().c_str());
        response.Init(srcDir, request.GetPath(), request.IsKeepAlive(), 200);
    } else {
        response.Init(srcDir, request.GetPath(), false, 400);
    }


    response.MakeResponse(writeBuff);
    /* 响应头 */
    iov[0].iov_base = const_cast<char*>(writeBuff.Peek());
    iov[0].iov_len = writeBuff.ReadableBytes();
    iovCnt = 1;

    /* 文件 */
    if(response.FileLen() > 0  && response.File()) {
        iov[1].iov_base = response.File();
        iov[1].iov_len = response.FileLen();
        iovCnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response.FileLen() , iovCnt, ToWriteBytes());
    return true;
}

void HttpConn::Close() {
    response.UnmapFile();
    if(m_isClose == false){
        m_isClose = true;
        m_user_count--;
        close(m_sockFd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", m_sockFd, GetIP(), GetPort(), (int)m_user_count);
    }
}

int HttpConn::GetFd() const {
    return m_sockFd;
}

int HttpConn::GetPort() const {
    return m_address.sin_port;
}

const char *HttpConn::GetIP() const {
    return inet_ntoa( m_address.sin_addr );
}

sockaddr_in HttpConn::GetAddr() const {
    return m_address;
}