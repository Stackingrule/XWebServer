cmake_minimum_required(VERSION 3.22)
project(XWebServer)

set(CMAKE_CXX_STANDARD 14)

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/buffer)
include_directories(${PROJECT_SOURCE_DIR}/include/http)
include_directories(${PROJECT_SOURCE_DIR}/include/locker)
include_directories(${PROJECT_SOURCE_DIR}/include/log)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/thread_pool)
include_directories(${PROJECT_SOURCE_DIR}/include/timer)
include_directories(${PROJECT_SOURCE_DIR}/include/util)

# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含了所有的相关代码
add_subdirectory(src)

add_executable(XWebServer XWebServer.cpp)
target_link_libraries(XWebServer HttpServer)