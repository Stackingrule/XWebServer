#ifndef XWEBSERVER_HTTP_REQUEST_HPP
#define XWEBSERVER_HTTP_REQUEST_HPP

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>

#include "buffer.hpp"

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

public:
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();
    bool ParseRequest(Buffer& buff);

    std::string GetPath() const;
    std::string& GetPath();
    std::string GetMethod() const;
    std::string GetVersion() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);

    void ParsePath();
    void ParsePost();
    void ParseFromUrlencoded();

private:
    PARSE_STATE state;  // 解析的状态
    std::string m_method, m_path, m_version, m_body; // http请求的方法、url、http版本、请求体
    std::unordered_map<std::string, std::string> m_header;  // 请求体的key-value
    std::unordered_map<std::string, std::string> m_post;    //

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);

};

#endif //XWEBSERVER_HTTP_REQUEST_HPP
