cmake_minimum_required(VERSION 3.10)
project(gateway_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加多个源文件
set(SOURCES
    gateway_server.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/base.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/file.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/gateway.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/user.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/friend.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/message.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/notify.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/speech.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/transmite.pb.cc
)

add_executable(gateway_server ${SOURCES})
# 添加标准路径中的第三方库

target_link_libraries(gateway_server -lgflags 
    -lspdlog -lfmt -lbrpc -lssl -lcrypto 
    -lprotobuf -lleveldb -letcd-cpp-api 
    -lodb-mysql -lodb -lodb-boost
    -lhiredis -lredis++
    -lcpprest -lcurl
    -lpthread -lboost_system)


include_directories(../common ../proto/cpp_out)