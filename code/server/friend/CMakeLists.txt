cmake_minimum_required(VERSION 3.10)
project(friend_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加多个源文件
set(SOURCES
    friend_server.cc
    ../proto/cpp_out/base.pb.cc
    ../proto/cpp_out/friend.pb.cc
    ../proto/cpp_out/message.pb.cc
    ../proto/cpp_out/user.pb.cc
)

add_executable(friend_server ${SOURCES})
# 添加标准路径中的第三方库

target_link_libraries(friend_server -ljsoncpp -lgflags 
    -lspdlog -lfmt -lbrpc -lssl -lcrypto 
    -lprotobuf -lleveldb -letcd-cpp-api 
    -lcpprest -lcurl -lmysqlclient -lodb-boost
    -lamqpcpp -lev -lelasticlient)


include_directories(../common ../proto/cpp_out)