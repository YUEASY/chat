#pragma once
#include <iostream>
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <etcd/Value.hpp>
#include <etcd/Watcher.hpp>
#include "logger.hpp"

namespace chat_ns
{
class Registry
{
public:
    using ptr = std::shared_ptr<Registry>;
    Registry(const std::string &host)
        : _client(std::make_shared<etcd::Client>(host)),
          _keep_alive(_client->leasekeepalive(3).get()),
          _lease_id(_keep_alive->Lease()) {}

    ~Registry()
    {
        _keep_alive->Cancel();
    }
    bool registry(const std::string &key, const std::string &val)
    {
        auto resp = _client->put(key, val, _lease_id).get();
        if (resp.is_ok())
        {
            LOG_DEBUG("注册数据成功 {}:{}", key, val);
        }
        else
        {
            LOG_ERROR("注册数据失败：{}", resp.error_message());
            return false;
        }
        return true;
    }

private:
    std::shared_ptr<etcd::Client> _client;
    std::shared_ptr<etcd::KeepAlive> _keep_alive;
    uint64_t _lease_id;
};

class Discovery
{
public:
    using NotifyCallback = std::function<void(const std::string &, const std::string &)>;
    Discovery(const std::string &host, const std::string &basedir, const NotifyCallback &put_cb, const NotifyCallback &del_cb)
        : _client(std::make_shared<etcd::Client>(host)),
          _watcher(std::make_shared<etcd::Watcher>(*_client.get(), basedir, std::bind(&Discovery::callback, this, std::placeholders::_1), true)),
          _put_cb(put_cb),
          _del_cb(del_cb)
    {
        auto resp = _client->get(basedir).get();
        if (!resp.is_ok())
        {
            LOG_ERROR("获取目录失败：{}", resp.error_message());
            return;
        }
        int sz = resp.keys().size();
        for (int i = 0; i < sz; i++)
        {
            LOG_DEBUG("发现服务：{}-{}", resp.value(i).as_string(), resp.key(i));
            if (_put_cb)
                _put_cb(resp.key(i), resp.value(i).as_string());
        }
        // _watcher = std::make_shared<etcd::Watcher>(*_client.get(), basedir, std::bind(&Discovery::callback, this, std::placeholders::_1), true);
    }

private:
    void callback(const etcd::Response &resp)
    {
        if (!resp.is_ok())
        {
            LOG_ERROR("收到错误的事件通知{}", resp.error_message());
            return;
        }
        for (const auto &ev : resp.events())
        {
            if (ev.event_type() == etcd::Event::EventType::PUT)
            {
                if (_put_cb)
                    _put_cb(ev.kv().key(), ev.kv().as_string());
                LOG_INFO("新增服务：{}-{}", ev.kv().key(), ev.kv().as_string());
            }
            else if (ev.event_type() == etcd::Event::EventType::DELETE_)
            {
                if (_del_cb)
                    _del_cb(ev.prev_kv().key(), ev.prev_kv().as_string());
                LOG_INFO("下线服务：{}-{}", ev.kv().key(), ev.kv().as_string());
            }
        }
    }

private:
    NotifyCallback _put_cb;
    NotifyCallback _del_cb;
    std::shared_ptr<etcd::Client> _client;
    std::shared_ptr<etcd::Watcher> _watcher;
};
}