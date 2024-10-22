cmake_minimum_required(VERSION 3.10)
project(message_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加多个源文件
set(SOURCES
    message_server.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/base.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/file.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/message.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/user.pb.cc
)

add_executable(message_server ${SOURCES})
# 添加标准路径中的第三方库

target_link_libraries(message_server -ljsoncpp -lgflags 
    -lspdlog -lfmt -lbrpc -lssl -lcrypto 
    -lprotobuf -lleveldb -letcd-cpp-api 
    -lcpprest -lcurl -lmysqlclient -lodb-boost
    -lamqpcpp -lev -lelasticlient)


include_directories(../common ../proto/cpp_out)