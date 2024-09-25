#pragma once
#include <brpc/channel.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "logger.hpp"

namespace chat_ns
{
    class ServiceChannel
    {

    public:
        using ChannelPtr = std::shared_ptr<brpc::Channel>;
        using ptr = std::shared_ptr<ServiceChannel>;
        ServiceChannel(const std::string &name)
            : _service_name(name),
              _index(0) {}
        void append(const std::string &host)
        {
            auto channel = std::make_shared<brpc::Channel>();
            brpc::ChannelOptions options;
            options.connect_timeout_ms = -1;
            options.timeout_ms = -1;
            options.max_retry = 3;
            options.protocol = "baidu_std";
            int ret = channel->Init(host.c_str(), &options);
            if (ret == -1)
            {
                LOG_ERROR("初始化{}-{}信道失败！", _service_name, host);
                return;
            }
            std::lock_guard<std::mutex> lock(_mutex);
            _hosts.insert(std::make_pair(host, channel));
            _channels.push_back(channel);
        }
        void remove(const std::string &host)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _hosts.find(host);
            if (it == _hosts.end())
            {
                LOG_WARN("{}-{}节点删除信道时，未找到相关信道信息！", _service_name, host);
                return;
            }
            for (auto vit = _channels.begin(); vit != _channels.end(); vit++)
            {
                if (*vit == it->second)
                {
                    _channels.erase(vit);
                    break;
                }
            }
            _hosts.erase(it);
        }
        // 通过RR轮转，获取一个Channel用于发起对应服务的rpc调用
        ChannelPtr choose()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_channels.size() == 0)
            {
                LOG_ERROR("当前没有能够提供{}服务的节点！", _service_name);
                return nullptr;
            }
            int32_t index = _index++ % _channels.size();
            return _channels[index];
        }

    private:
        std::mutex _mutex;
        int32_t _index;                                     // 当前轮转下标计数器
        std::string _service_name;                          // 服务名
        std::vector<ChannelPtr> _channels;                  // 当前服务对应的信道集合
        std::unordered_map<std::string, ChannelPtr> _hosts; // 主机地址与信道的映射关系
    };

    class ServiceManager
    {

    public:
        using ptr = std::shared_ptr<ServiceManager>;
        using ChannelPtr = std::shared_ptr<brpc::Channel>;
        ChannelPtr choose(const std::string &service_name)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto sit = _services.find(service_name);
            if (sit == _services.end())
            {
                LOG_ERROR("当前没有能够提供{}服务的节点！", service_name);
                return nullptr;
            }
            return sit->second->choose();
        }
        void declared(const std::string &service_name)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _follow_services.insert(service_name);
        }
        void onServiceOnline(const std::string &service_instance, const std::string &host)
        {
            std::string service_name = getServiceName(service_instance);
            ServiceChannel::ptr service;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                auto fit = _follow_services.find(service_name);
                if (fit == _follow_services.end())
                {
                    LOG_DEBUG("{}-{}服务已上线(未关心)", service_name, host);
                    return;
                }
                auto sit = _services.find(service_name);
                if (sit == _services.end())
                {
                    service = std::make_shared<ServiceChannel>(service_name);
                    _services.insert(std::make_pair(service_name, service));
                }
                else
                {
                    service = sit->second;
                }
            }
            if (!service)
            {
                LOG_ERROR("新增{}服务管理节点失败！", service_name);
                return;
            }
            service->append(host);
            LOG_INFO("{}-{}服务已上线，已添加至管理", service_name, host);
        }
        void onServiceOffline(const std::string &service_instance, const std::string &host)
        {
            std::string service_name = getServiceName(service_instance);
            ServiceChannel::ptr service;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                auto fit = _follow_services.find(service_name);
                if (fit == _follow_services.end())
                {
                    LOG_DEBUG("{}-{}服务已下线(未关心)", service_name, host);
                    return;
                }
                auto sit = _services.find(service_name);
                if (sit == _services.end())
                {
                    LOG_WARN("删除{}服务管理节点时，找不到管理对象！", service_name);
                    return;
                }
                service = sit->second;
            }
            service->remove(host);
            LOG_INFO("{}-{}服务已下线，已在管理中删除", service_name, host);
        }

    private:
        std::string getServiceName(const std::string &service_instance)
        {
            auto pos = service_instance.find_last_of('/');
            if (pos == std::string::npos)
                return service_instance;
            return service_instance.substr(0, pos);
        }

    private:
        std::mutex _mutex;
        std::unordered_set<std::string> _follow_services;
        std::unordered_map<std::string, ServiceChannel::ptr> _services;
    };
}