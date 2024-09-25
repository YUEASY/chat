cmake_minimum_required(VERSION 3.10)
project(speech_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 开启警告
add_compile_options(-Wall)

# 添加多个源文件
set(SOURCES
    speech_server.cc
    speech.pb.cc
)

add_executable(speech_server ${SOURCES})
# 添加标准路径中的第三方库
target_link_libraries(speech_server -lgflags -ljsoncpp -lbrpc -lspdlog -lfmt -lprotobuf -lleveldb -letcd-cpp-api -lcpprest -lcurl -lssl -lcrypto)
