#include "http_response.hpp"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
        { ".html",  "text/html" },
        { ".xml",   "text/xml" },
        { ".xhtml", "application/xhtml+xml" },
        { ".txt",   "text/plain" },
        { ".rtf",   "application/rtf" },
        { ".pdf",   "application/pdf" },
        { ".word",  "application/nsword" },
        { ".png",   "image/png" },
        { ".gif",   "image/gif" },
        { ".jpg",   "image/jpeg" },
        { ".jpeg",  "image/jpeg" },
        { ".au",    "audio/basic" },
        { ".mpeg",  "video/mpeg" },
        { ".mpg",   "video/mpeg" },
        { ".avi",   "video/x-msvideo" },
        { ".gz",    "application/x-gzip" },
        { ".tar",   "application/x-tar" },
        { ".css",   "text/css "},
        { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
        { 200, "OK" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
        { 400, "/400.html" },
        { 403, "/403.html" },
        { 404, "/404.html" },
};

HttpResponse::HttpResponse() {
    res_code = -1;
    m_path = m_srcDir = "";
    m_isKeepAlive = false;
    mmFile = nullptr;
    mmFileStat = { 0 };
}

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const std::string &srcDir, std::string &path, bool isKeepAlive, int code) {
    assert(srcDir != "");
    if(mmFile) {
        UnmapFile();
    }
    res_code = code;
    m_isKeepAlive = isKeepAlive;
    m_path = path;
    m_srcDir = srcDir;
    mmFile = nullptr;
    mmFileStat = { 0 };
}


void HttpResponse::MakeResponse(Buffer &buff) {
    /* 判断请求的资源文件 */
    if(stat((m_srcDir + m_path).data(), &mmFileStat) < 0 || S_ISDIR(mmFileStat.st_mode)) {
        res_code = 404;
    }
    else if(!(mmFileStat.st_mode & S_IROTH)) {
        res_code = 403;
    }
    else if(res_code == -1) {
        res_code = 200;
    }
    ErrorHtml();
    AddStateLine(buff);
    AddHeader(buff);
    AddContent(buff);
}

void HttpResponse::AddStateLine(Buffer &buff) {
    std::string status;
    if(CODE_STATUS.count(res_code) == 1) {
        status = CODE_STATUS.find(res_code)->second;
    }
    else {
        res_code = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(res_code) + " " + status + "\r\n");
}

void HttpResponse::AddHeader(Buffer &buff) {
    buff.Append("Connection: ");
    if(m_isKeepAlive) {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    } else{
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType() + "\r\n");
}

void HttpResponse::AddContent(Buffer &buff) {
    int srcFd = open((m_srcDir + m_path).data(), O_RDONLY);
    if(srcFd < 0) {
        ErrorContent(buff, "File NotFound!");
        return;
    }

    /* 将文件映射到内存提高文件的访问速度
        MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    LOG_DEBUG("file path %s", (m_srcDir + m_path).data());
    int* mmRet = (int*)mmap(0, mmFileStat.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1) {
        ErrorContent(buff, "File NotFound!");
        return;
    }
    mmFile = (char*)mmRet;
    close(srcFd);
    buff.Append("Content-length: " + std::to_string(mmFileStat.st_size) + "\r\n\r\n");
}

void HttpResponse::UnmapFile() {
    if(mmFile) {
        munmap(mmFile, mmFileStat.st_size);
        mmFile = nullptr;
    }
}

char *HttpResponse::File() {
    return mmFile;
}

size_t HttpResponse::FileLen() const {
    return mmFileStat.st_size;
}

void HttpResponse::ErrorHtml() {}

std::string HttpResponse::GetFileType() {
    /* 判断文件类型 */
    std::string::size_type idx = m_path.find_last_of('.');
    if(idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = m_path.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::ErrorContent(Buffer& buff, std::string message) {
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(res_code == 1)) {
        status = CODE_STATUS.find(res_code)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(res_code) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>XWebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}
