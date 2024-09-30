cmake_minimum_required(VERSION 3.10)
project(user_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加多个源文件
set(SOURCES
    user_server.cc
    ../proto/cpp_out/base.pb.cc
    ../proto/cpp_out/file.pb.cc
    ../proto/cpp_out/user.pb.cc
)

add_executable(user_server ${SOURCES})
# 添加标准路径中的第三方库
target_link_libraries(user_server -ljsoncpp  -pthread -lgtest -lgflags -lspdlog -lfmt -lbrpc -lssl -lcrypto -lprotobuf -lleveldb -letcd-cpp-api -lcpprest -lcurl -lcpr -lelasticlient -lhiredis -lredis++ -lmysqlclient)
