#include "epoller.hpp"


Epoller::Epoller(int maxEvent):epollFd(epoll_create(512)), eventsList(maxEvent){
    assert(epollFd >= 0 && eventsList.size() > 0);
}

Epoller::~Epoller() {
    close(epollFd);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;

    ev.events = events;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Wait(int timeoutMs) {
    return epoll_wait(epollFd, &eventsList[0], static_cast<int>(eventsList.size()), timeoutMs);
}

int Epoller::GetEventFd(size_t i) const {
    assert(i < eventsList.size() && i >= 0);
    return eventsList[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < eventsList.size() && i >= 0);
    return eventsList[i].events;
}