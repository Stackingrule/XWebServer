# XWebServer

[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)
## Introduction

这是一个基于 C++11 实现的简单的 HTTP Web 服务器，目前支持GET方法处理静态资源。并发模型采用: 单进程＋Reactor+非阻塞方式运行。支持HTTP长连接，并实现了异步日志，用于记录服务器运行状态。

## Environment
+ OS: Ubuntu 18.04

+ 编译器: g++ 9.4.0

+ 版本控制: git

+ 自动化构建: CMake 3.22.1

+ 集成开发工具: CLion

+ 压测工具：[WebBench](https://github.com/EZLippi/WebBench)

## Build

	./autobuild.sh

## Usage
	./XWebServer

## Technical points
* 使用epoll边沿触发的IO多路复用技术，非阻塞IO，以及线程池实现多线程的Reactor高并发模型
* 使用多线程充分利用多核CPU，并使用线程池避免线程频繁创建销毁的开销
* 基于小根堆实现的定时器，关闭超时的非活动连接
* 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
* 使用双缓冲区、单例模式与阻塞队列技术实现了简单的异步日志系统
* 使用状态机解析了HTTP请求
* 支持优雅关闭连接；支持 HTTP 长连接
* 利用单例模式与阻塞队列实现异步的日志系统，记录服务器运行状态
* 经Webbench压力测试可以实现上万的并发连接数据交换


## TODO
- [ ] config配置
- [ ] 完善单元测试
- [ ] 实现循环缓冲区
- [ ] 反向代理和负载均衡
