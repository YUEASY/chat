cmake_minimum_required(VERSION 3.10)
project(file_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 开启警告
add_compile_options(-Wall)

# 添加多个源文件
set(SOURCES
    file_server.cc
    ${CMAKE_SOURCE_DIR}/../proto/cpp_out/base.pb.cc
    ${CMAKE_SOURCE_DIR}/../proto/cpp_out/file.pb.cc
)

add_executable(file_server ${SOURCES})

# 添加标准路径中的第三方库
target_link_libraries(file_server
    gflags
    spdlog
    fmt
    brpc
    ssl
    crypto
    protobuf
    leveldb
    etcd-cpp-api
    cpprest
    curl
    jsoncpp
)

