#pragma once
#include <iostream>
#include <functional>
#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <openssl/ssl.h>
#include <openssl/opensslv.h>
#include "logger.hpp"
namespace chat_ns
{
#define ROUTING_KEY "default_routing_key"
class MQClient
{
    using MessageCallback = std::function<void(const char *, size_t)>;

public:
    MQClient(std::string_view user, std::string_view passwd, std::string_view host)
    {
        _loop = EV_DEFAULT;
        _handler = std::make_unique<AMQP::LibEvHandler>(_loop);
        std::string url = std::string("amqp://").append(user).append(":").append(passwd).append("@").append(host).append("/");
        _connection = std::make_unique<AMQP::TcpConnection>(_handler.get(), AMQP::Address(url));
        _channel = std::make_unique<AMQP::TcpChannel>(_connection.get());
        _loop_thread = std::thread([this]()
                                   { ev_run(_loop); });
    }
    ~MQClient()
    {
        ev_async async_watcher;
        ev_async_init(&async_watcher, warcher_callback);
        ev_async_start(_loop, &async_watcher);
        ev_async_send(_loop, &async_watcher);
        _loop = nullptr;
        _loop_thread.join();
    }
    void declareComponents(std::string_view exchange, std::string_view queue,
                           std::string_view routing_key = ROUTING_KEY, AMQP::ExchangeType echange_type = AMQP::ExchangeType::direct)
    {
        _channel->declareExchange(exchange, echange_type)
            .onError([](const char *message)
                     {  LOG_ERROR("声明交换机失败：{}", message);exit(0); })
            .onSuccess([exchange]()
                       { LOG_DEBUG("交换机 {} 创建成功！", exchange); });
        _channel->declareQueue(queue)
            .onError([](const char *message)
                     { LOG_ERROR("声明队列失败：{}", message); exit(0); })
            .onSuccess([queue]()
                       { LOG_DEBUG("队列 {} 创建成功！", queue); });

        _channel->bindQueue("test-exchange", "test-queue", "test-queue-key")
            .onError([exchange, queue](const char *message)
                     { LOG_ERROR("{}-{}绑定失败：{}", exchange, queue, message); exit(0); })
            .onSuccess([exchange, queue]()
                       { LOG_DEBUG("{}-{}绑定成功！", exchange, queue); });
    }
    bool publish(std::string_view exchange, std::string_view msg, std::string_view routing_key = ROUTING_KEY)
    {
        bool ret = _channel->publish(exchange, routing_key, msg);
        if (ret == false)
        {
            LOG_ERROR("{}[{}]消息发送失败！", exchange, routing_key);
            return false;
        }
        return true;
    }
    void consume(std::string_view queue, const MessageCallback &callback, std::string_view tag = "consume-tag")
    {
        _channel->consume(queue, tag)
            .onReceived([=](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                        {callback(message.body(),message.size()); _channel->ack(deliveryTag); })
            .onError([queue](const char *message)
                     { LOG_ERROR("{}订阅失败：{}", queue,message);exit(0); })
            .onSuccess([queue]()
                       { LOG_DEBUG("{}订阅成功！", queue); });
    }

private:
    static void warcher_callback(struct ev_loop *loop, ev_async *watcher, int32_t revents)
    {
        ev_break(loop, EVBREAK_ALL);
    }

private:
    struct ev_loop *_loop;
    std::unique_ptr<AMQP::LibEvHandler> _handler;
    std::unique_ptr<AMQP::TcpConnection> _connection;
    std::unique_ptr<AMQP::TcpChannel> _channel;
    std::thread _loop_thread;
};
}