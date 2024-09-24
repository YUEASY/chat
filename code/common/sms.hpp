#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <iostream>
class SMS
{
public:
    SMS(std::string_view phone, std::string_view secret)
        : _phone(phone),
          _client("api.smsbao.com")
    {
        _content.append("【友聊聊天室】您的验证码是");
        _content.append(secret);
        _content.append("。五分钟内有效。");

        auto res = _client.Get("/sms?u=" + _name + "&p=" + _passwd + "&m=" + _phone + "&c=" + _content);
        // if (res && res->status == 200)
        // {
        //     // 处理成功
        // }
        // else
        // {
        //     // 处理错误
        // }
    }

private:
    std::string _name = "debuggerzero";                       
    std::string _passwd = "dbdcf36bd39d446f88a8162ba6f57c85"; 
    std::string _phone;
    std::string _content;
    httplib::SSLClient _client; 
};
