#include "log.hpp"
#include "http_request.hpp"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
        "/index", "/register", "/login",
        "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
        {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::Init() {
    m_method = m_path = m_version= m_body = "";
    state = REQUEST_LINE;
    m_header.clear();
    m_post.clear();
}

bool HttpRequest::ParseRequest(Buffer &buff) {
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0) {
        return false;
    }
    while(buff.ReadableBytes() && state != FINISH) {
        const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);
        switch(state)
        {
            case REQUEST_LINE:
                if(!ParseRequestLine(line)) {
                    return false;
                }
                ParsePath();
                break;
            case HEADERS:
                ParseHeader(line);
                if(buff.ReadableBytes() <= 2) {
                    state = FINISH;
                }
                break;
            case BODY:
                ParseBody(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", m_method.c_str(), m_path.c_str(), m_version.c_str())
    return true;
}

void HttpRequest::ParsePath() {
    if(m_path == "/") {
        m_path = "/index.html";
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == m_path) {
                m_path += ".html";
                break;
            }
        }
    }
}

bool HttpRequest::ParseRequestLine(const std::string &line) {
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        m_method = subMatch[1];
        m_path = subMatch[2];
        m_version = subMatch[3];
        state = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error")
    return false;
}

void HttpRequest::ParseHeader(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        m_header[subMatch[1]] = subMatch[2];
    }
    else {
        state = BODY;
    }
}

void HttpRequest::ParseBody(const std::string& line) {
    m_body = line;
    ParsePost();
    state = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size())
}

int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void HttpRequest::ParsePost() {
//    if(m_method == "POST" && m_header["Content-Type"] == "application/x-www-form-urlencoded") {
//        ParseFromUrlencoded();
//        if(DEFAULT_HTML_TAG.count(m_path)) {
//            int tag = DEFAULT_HTML_TAG.find(m_path)->second;
//            LOG_DEBUG("Tag:%d", tag);
//            if(tag == 0 || tag == 1) {
//                bool isLogin = (tag == 1);
//                if(UserVerify(m_post["username"], m_post["password"], isLogin)) {
//                    m_path = "/welcome.html";
//                }
//                else {
//                    m_path = "/error.html";
//                }
//            }
//        }
//    }

    return;
}

void HttpRequest::ParseFromUrlencoded() {
    if(m_body.size() == 0) {
        return;
    }

    std::string key, value;
    int num = 0;
    int n = m_body.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = m_body[i];
        switch (ch) {
            case '=':
                key = m_body.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                m_body[i] = ' ';
                break;
            case '%':
                num = ConverHex(m_body[i + 1]) * 16 + ConverHex(m_body[i + 2]);
                m_body[i + 2] = num % 10 + '0';
                m_body[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = m_body.substr(j, i - j);
                j = i + 1;
                m_post[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str())
                break;
            default:
                break;
        }
    }
    assert(j <= i);
    if(m_post.count(key) == 0 && j < i) {
        value = m_body.substr(j, i - j);
        m_post[key] = value;
    }
}

std::string HttpRequest::GetPath() const {
    return m_path;
}

std::string &HttpRequest::GetPath() {
    return m_path;
}

std::string HttpRequest::GetMethod() const {
    return m_method;
}

std::string HttpRequest::GetVersion() const {
    return m_method;
}

std::string HttpRequest::GetPost(const std::string &key) const {
    assert(key != "");
    if(m_post.count(key) == 1) {
        return m_post.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char *key) const {
    assert(key != nullptr);
    if(m_post.count(key) == 1) {
        return m_post.find(key)->second;
    }
    return "";
}

bool HttpRequest::IsKeepAlive() const {
    if(m_header.count("Connection") == 1) {
        return m_header.find("Connection")->second == "keep-alive" && m_version == "1.1";
    }
    return false;
}
