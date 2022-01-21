#ifndef XWEBSERVER_BUFFER_HPP
#define XWEBSERVER_BUFFER_HPP

//#include <string>
//#include <cstring>   //perror
//#include <iostream>
//#include <unistd.h>  // write
//#include <sys/uio.h> //readv
//#include <vector> //readv
//#include <atomic>
//#include <regex>
//#include <cassert>
//
////#define INIT_SIZE 1024
//static const size_t INIT_SIZE = 1024;
////static const char CRLF[] = "\r\n";
//
//class Buffer {
//public:
//    Buffer();
//    ~Buffer() = default;
//
//    size_t WritableBytes() const;       // 可写字节
//    size_t ReadableBytes() const ;      // 可读字节
//    size_t PrependableBytes() const;    // 先前字节数
//
//    const char* PeekConst() const;
//    char* Peek();
//
//    void EnsureWriteable(size_t len);   //确保空间足够
//    void HasWritten(size_t len);        //调整可写位置
//
//    void RetrieveLen(size_t len);               // 取len字节的数据
//    void RetrieveToEnd(const char* end);        //一直取出字节直到end
//
//    void RetrieveAll() ;                    // 取出全部
//    std::string RetrieveAllToString();
//
//    const char* BeginWriteConst() const;
//    char* BeginWrite();                     // 开始写的位置
//
//    void Append(const std::string& str);
//    void Append(const char* str, size_t len);
//    void Append(const void* data, size_t len);
//    void Append(const Buffer& buff);
//
//    ssize_t ReadFd(int fd, int* savedErrno);
//    ssize_t WriteFd(int fd, int* Errno);
//
//    const char* FindCRLF() const;
//    const char* FindCRLF(const char* start) const;
//
//private:
//    char* BeginPtr();               // 返回buffer起始地址
//    const char* BeginPtr() const;   // 返回buffer起始地址
//    void ResizeSpace(size_t len);   // 调整Buffer大小
//
//    std::vector<char> m_buffer;
//    std::atomic<std::size_t> read_pos;  // 读下标
//    std::atomic<std::size_t> write_pos; // 写下标
//};


#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>

class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const ;
    size_t PrependableBytes() const;

    const char* Peek() const;   //第一个可读位置
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);  // 从buffer中取出len字节
    void RetrieveUntil(const char* end);

    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr();
    const char* BeginPtr() const;
    void MakeSpace(size_t len);

    std::vector<char> buffer;
    std::atomic<std::size_t> readPos;
    std::atomic<std::size_t> writePos;
};

#endif //XWEBSERVER_BUFFER_HPP
