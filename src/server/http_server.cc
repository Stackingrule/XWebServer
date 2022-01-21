#include "http_server.hpp"

HttpServer::HttpServer(
        int port, int trigMode, int timeoutMS,
        bool OptLinger,
        int threadNum,
        bool openLog, int logLevel, int logQueSize):

        m_port(port), openLinger(OptLinger), timeoutMS_(timeoutMS), isClose(false),
        m_timer(new HeapTimer()), threadpool(new ThreadPool(threadNum)), m_epoller(new Epoller())
{
    m_srcDir = getcwd(nullptr, 256);

    assert(m_srcDir);
    strncat(m_srcDir, "/resources/", 16);

    std::cout << m_srcDir << std::endl;

    HttpConn::m_user_count = 0;
    HttpConn::srcDir = m_srcDir;

    InitEventMode(trigMode);
    if(!InitSocket()) { isClose = true;}

    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", m_port, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                     (listenEvent & EPOLLET ? "ET": "LT"),
                     (connEvent & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
        }
    }
}

HttpServer::~HttpServer() {
    close(listenFd);
    isClose = true;
    free(m_srcDir);
}

void HttpServer::InitEventMode(int trigMode) {
    listenEvent = EPOLLRDHUP;
    connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode) {
        case 0:
            break;
        case 1:
            connEvent |= EPOLLET;
            break;
        case 2:
            listenEvent |= EPOLLET;
            break;
        case 3:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
            break;
        default:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
            break;
    }
    HttpConn::isET = (connEvent & EPOLLET);
}

void HttpServer::Start() {
    int timeMS = -1;  /* epoll wait timeout == -1 无事件将阻塞 */
    if(!isClose) { LOG_INFO("========== Server start =========="); }
    while(!isClose) {
        if(timeoutMS_ > 0) {
            timeMS = m_timer->GetNextTick();
        }
        int eventCnt = m_epoller->Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            /* 处理事件 */
            int fd = m_epoller->GetEventFd(i);
            uint32_t events = m_epoller->GetEvents(i);
            if(fd == listenFd) {
                DealListen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(m_users.count(fd) > 0);
                CloseConn(&m_users[fd]);
            }
            else if(events & EPOLLIN) {
                assert(m_users.count(fd) > 0);
                DealRead(&m_users[fd]);
            }
            else if(events & EPOLLOUT) {
                assert(m_users.count(fd) > 0);
                DealWrite(&m_users[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void HttpServer::SendError(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void HttpServer::CloseConn(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    m_epoller->DelFd(client->GetFd());
    client->Close();
}

void HttpServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    m_users[fd].Init(fd, addr);
    if(timeoutMS_ > 0) {
        m_timer->add(fd, timeoutMS_, std::bind(&HttpServer::CloseConn, this, &m_users[fd]));
    }
    m_epoller->AddFd(fd, EPOLLIN | connEvent);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", m_users[fd].GetFd());
}

void HttpServer::DealListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(HttpConn::m_user_count >= MAX_FD) {
            SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient(fd, addr);
    } while(listenEvent & EPOLLET);
}

void HttpServer::DealRead(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool->AddTask(std::bind(&HttpServer::OnRead, this, client));
}

void HttpServer::DealWrite(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool->AddTask(std::bind(&HttpServer::OnWrite, this, client));
}

void HttpServer::ExtentTime(HttpConn* client) {
    assert(client);
    if(timeoutMS_ > 0) { m_timer->adjust(client->GetFd(), timeoutMS_); }
}

void HttpServer::OnRead(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->Read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void HttpServer::OnProcess(HttpConn* client) {
    if(client->Process()) {
        m_epoller->ModFd(client->GetFd(), connEvent | EPOLLOUT);
    } else {
        m_epoller->ModFd(client->GetFd(), connEvent | EPOLLIN);
    }
}

void HttpServer::OnWrite(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->Write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        /* 传输完成 */
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            m_epoller->ModFd(client->GetFd(), connEvent | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}

/* Create listenFd */
bool HttpServer::InitSocket() {
    int ret;
    struct sockaddr_in addr;
    if(m_port > 65535 || m_port < 1024) {
        LOG_ERROR("Port:%d error!",  m_port);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);
    struct linger optLinger = { 0 };
    if(openLinger) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0) {
        LOG_ERROR("Create socket error!", m_port);
        return false;
    }

    ret = setsockopt(listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd);
        LOG_ERROR("Init linger error!", m_port);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd);
        return false;
    }

    ret = bind(listenFd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", m_port);
        close(listenFd);
        return false;
    }

    ret = listen(listenFd, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", m_port);
        close(listenFd);
        return false;
    }
    ret = m_epoller->AddFd(listenFd,  listenEvent | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd);
        return false;
    }
    SetFdNonblock(listenFd);
    LOG_INFO("Server port:%d", m_port);
    return true;
}

int HttpServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}


