set(
        SRC_LIST
        ./buffer/buffer.cc
        ./log/log.cc
        ./http/http_conn.cc
        ./http/http_request.cc
        ./http/http_response.cc
        ./timer/timer.cc
        ./server/epoller.cc
        ./server/http_server.cc
)
add_library(HttpServer ${SRC_LIST})
target_link_libraries(HttpServer pthread)