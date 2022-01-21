#ifndef XWEBSERVER_HTTP_RESPONSE_HPP
#define XWEBSERVER_HTTP_RESPONSE_HPP

#include <unordered_map>
#include <fcntl.h>          // open
#include <unistd.h>         // close
#include <sys/stat.h>       // stat
#include <sys/mman.h>       // mmap, munmap

#include "buffer.hpp"
#include "log.hpp"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int GetResCode() const { return res_code; }

private:
    void AddStateLine(Buffer &buff);
    void AddHeader(Buffer &buff);
    void AddContent(Buffer &buff);

    void ErrorHtml();
    std::string GetFileType();

    int res_code;       // 响应码
    bool m_isKeepAlive; //

    std::string m_path;
    std::string m_srcDir;

    char* mmFile;
    struct stat mmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;

};

#endif //XWEBSERVER_HTTP_RESPONSE_HPP
