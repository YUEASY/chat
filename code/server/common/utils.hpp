#include <iostream>
#include <fstream>
#include <sstream>
#include <atomic>
#include <random>
#include <chrono>
#include <iomanip>
#include "logger.hpp"
namespace chat_ns
{
    class Utils
    {
    public:
        static std::string uuid()
        {
            // 生成前8位随机字母数字
            const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            std::string randomPart;
            std::random_device rd;
            std::default_random_engine generator(rd());
            std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);

            for (int i = 0; i < 8; ++i)
            {
                randomPart += charset[distribution(generator)];
            }

            // 获取当前时间
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_c);

            // 生成后8位（日、时、分、秒）
            std::ostringstream timePart;
            timePart << std::setw(2) << now_tm.tm_mday
                     << std::setw(2) << now_tm.tm_hour
                     << std::setw(2) << now_tm.tm_min
                     << std::setw(2) << now_tm.tm_sec;

            return randomPart + timePart.str();
        }
  
        static bool readFile(std::string_view filename, std::string &body)
        {
            std::ifstream file(filename.data(),std::ios::binary | std::ios::in);
            if (!file.is_open())
            {
                LOG_ERROR("打开文件{}失败",filename.data());
                return false; 
            }

            // body.assign((std::istreambuf_iterator<char>(file)),
            //             std::istreambuf_iterator<char>());
            file.seekg(0,std::ios::end);
            size_t flen = file.tellg();
            file.seekg(0,std::ios::beg);
            body.resize(flen);
            file.read(&body[0],flen);
            if(file.good() == false)
            {
                LOG_ERROR("读取文件{}失败",filename.data());
                file.close();
                return false;
            }
            file.close();
            return true; 
        }
        static bool writeFile(std::string_view filename, const std::string &body)
        {
            std::ofstream file(filename.data(), std::ios::binary | std::ios::out | std::ios::trunc);
            if (!file.is_open())
            {
                return false; 
            }

            file << body;
            if (file.good() == false)
            {
                LOG_ERROR("写入文件{}失败", filename.data());
                file.close();
                return false;
            }
            file.close();
            return true;  
        }
    };

}