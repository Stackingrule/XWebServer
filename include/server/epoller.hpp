#ifndef XWEBSERVER_EPOLLER_HPP
#define XWEBSERVER_EPOLLER_HPP

#include <sys/epoll.h>  // epoll_ctl()
#include <fcntl.h>      // fcntl()
#include <unistd.h>     // close()
#include <cassert>      // close()
#include <vector>
#include <cerrno>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();

    bool AddFd(int fd, uint32_t events);

    bool ModFd(int fd, uint32_t events);

    bool DelFd(int fd);

    int Wait(int timeoutMs = -1);

    int GetEventFd(size_t i) const;

    uint32_t GetEvents(size_t i) const;

private:
    int epollFd;

    std::vector<struct epoll_event> eventsList;
};
#endif //XWEBSERVER_EPOLLER_HPP
