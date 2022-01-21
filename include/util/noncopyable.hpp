#ifndef XWEBSERVER_NONCOPYABLE_HPP
#define XWEBSERVER_NONCOPYABLE_HPP

class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

};

#endif //XWEBSERVER_NONCOPYABLE_HPP
