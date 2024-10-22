cmake_minimum_required(VERSION 3.10)
project(transmite_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 开启警告
# add_compile_options(-Wall)

# 添加多个源文件
set(SOURCES
    transmite_server.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/base.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/user.pb.cc
    ${CMAKE_SOURCE_DIR}/proto/cpp_out/transmite.pb.cc
)

add_executable(transmite_server ${SOURCES})
# 添加标准路径中的第三方库
target_link_libraries(transmite_server -ljsoncpp -lgflags 
    -lspdlog -lfmt -lbrpc -lssl -lcrypto 
    -lprotobuf -lleveldb -letcd-cpp-api 
    -lcpprest -lcurl -lmysqlclient -lodb-boost
    -lamqpcpp -lev)

include_directories(../common ../proto/cpp_out)