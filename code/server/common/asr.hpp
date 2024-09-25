#pragma once
#include "../third/aip-cpp-sdk-4.16.7/speech.h"
#include "logger.hpp"
namespace chat_ns
{
    class ASR
    {
    public:
        ASR(const std::string &speech_data,
            const std::string &app_id = "115671091",
            const std::string &api_key = "VhJ3vmvLdwI45IJR4msPDMoW",
            const std::string &secret_key = "YLgWnHCS2grOPBT6c0b3wqrSlDGIA4Gh")
            : _client(app_id, api_key, secret_key)
        {
            Json::Value result = _client.recognize(speech_data, "pcm", 16000, aip::null);

            if (result["err_no"].asInt() == 0)
            {
                _str = result["result"][0].asString();
            }
            else
            {
                LOG_ERROR("语音识别转换失败：{}", result["err_msg"].asString());
                _str = "";
            }
        }
        std::string tostr()
        {
            return _str;
        }

    private:
        aip::Speech _client;
        std::string _str;
    };
}