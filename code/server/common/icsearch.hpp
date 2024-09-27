#pragma once
#include <elasticlient/client.h>
#include <cpr/cpr.h>
#include <json/json.h>
#include <iostream>
#include <memory>
#include "logger.hpp"
namespace chat_ns
{
class JsonSerializer
{
public:
    // 序列化
    static std::string serialize(const Json::Value &value)
    {
        Json::StreamWriterBuilder writer;
        std::ostringstream os;
        std::unique_ptr<Json::StreamWriter> jsonWriter(writer.newStreamWriter());
        jsonWriter->write(value, &os);
        return os.str();
    }

    // 反序列化
    static Json::Value deserialize(const std::string &jsonString)
    {
        std::istringstream is(jsonString);
        Json::CharReaderBuilder reader;
        Json::Value root;
        std::string errs;
        Json::parseFromStream(reader, is, &root, &errs);
        return root;
    }
};

class ESIndex
{
public:
    ESIndex(const std::shared_ptr<elasticlient::Client> &client, const std::string &name, const std::string &type = "_doc")
        : _client(client), _name(name), _type(type)
    {
        Json::Value analysis;
        Json::Value analyzer;
        Json::Value ik;
        Json::Value tokenizer;
        tokenizer["tokenizer"] = "ik_max_word";
        ik["ik"] = tokenizer;
        analyzer["analyzer"] = ik;
        analysis["analysis"] = analyzer;
        _index["settings"] = analysis;
    }
    ESIndex &append(const std::string &key, const std::string &type = "text", const std::string &analyzer = "ik_max_word", bool enabled = true)
    {
        Json::Value fields;
        fields["type"] = type;
        fields["analyzer"] = analyzer;
        if (enabled == false)
            fields["enabled"] = enabled;
        _properties[key] = fields;
        return *this;
    }
    bool create(const std::string &index_id = "default_index_id")
    {
        Json::Value mappings;
        mappings["dynamic"] = true;
        mappings["properties"] = _properties;
        _index["mappings"] = mappings;

        std::string body;
        body = JsonSerializer::serialize(_index);
        try
        {
            auto rsp = _client->index(_name, _type, index_id, body);
            if (rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("创建ES索引{}失败，响应状态码：{}", _name, rsp.status_code);
                return false;
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("创建ES索引{}失败：{}", _name, e.what());
            return false;
        }
        LOG_DEBUG("创建ES索引{}成功", _name);
        return true;
    }

private:
    std::string _name;
    std::string _type;
    Json::Value _index;
    Json::Value _properties;
    std::shared_ptr<elasticlient::Client> _client;
};

class ESInsert
{
public:
    ESInsert(std::shared_ptr<elasticlient::Client> &client, const std::string &name, const std::string &type = "_doc")
        : _name(name), _type(type), _client(client)
    {
    }
    ESInsert &append(const std::string &key, const std::string &val)
    {
        _item[key] = val;
        return *this;
    }
    bool insert(const std::string &id = "")
    {
        std::string body;
        body = JsonSerializer::serialize(_item);
        try
        {
            auto rsp = _client->index(_name, _type, id, body);
            if (rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("插入数据{}失败，响应状态码：{}", _name, rsp.status_code);
                return false;
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("插入数据{}失败：{}", _name, e.what());
            return false;
        }
        LOG_DEBUG("插入数据{}成功", _name);
        return true;
    }

private:
    std::string _name;
    std::string _type;
    Json::Value _item;
    std::shared_ptr<elasticlient::Client> _client;
};

class ESRemove
{
public:
    ESRemove(std::shared_ptr<elasticlient::Client> &client, const std::string &name, const std::string &type = "_doc")
        : _name(name), _type(type), _client(client) {}
    bool remove(const std::string &id)
    {
        try
        {
            auto rsp = _client->remove(_name, _type, id);
            if (rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("删除数据{}失败，响应状态码：{}", id, rsp.status_code);
                return false;
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("删除数据{}失败：{}", id, e.what());
            return false;
        }
        LOG_DEBUG("删除数据{}成功", id);
        return true;
    }

private:
    std::string _name;
    std::string _type;
    std::shared_ptr<elasticlient::Client> _client;
};

class ESSearch
{

public:
    ESSearch(std::shared_ptr<elasticlient::Client> &client, const std::string &name, const std::string &type = "_doc")
        : _name(name), _type(type), _client(client) {}

    ESSearch &appendMustNotTerm(const std::string &key, const std::vector<std::string> &vals)
    {
        Json::Value fields;
        for (const auto &val : vals)
        {
            fields[key].append(val);
        }
        Json::Value terms;
        terms["terms"] = fields;
        _must_not.append(terms);
        return *this;
    }
    ESSearch &appendShouldMatch(const std::string &key, const std::string &val)
    {
        Json::Value field;
        field[key] = val;
        Json::Value match;
        match["match"] = field;
        _should.append(match);
        return *this;
    }
    Json::Value search()
    {
        Json::Value cond;
        if (!_must_not.empty())
            cond["must_not"] = _must_not;
        if (!_should.empty())
            cond["should"] = _should;
        Json::Value query;
        Json::Value root;
        query["bool"] = cond;
        root["query"] = query;
        std::string body;
        body = JsonSerializer::serialize(root);
        cpr::Response rsp;
        try
        {
            rsp = _client->search(_name, _type, body);
            if (rsp.status_code < 200 || rsp.status_code >= 300)
            {
                LOG_ERROR("检索数据{}失败，响应状态码：{}", _name, rsp.status_code);
                return Json::Value();
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("检索数据{}失败：{}", _name, e.what());
            return Json::Value();
        }
        return JsonSerializer::deserialize(rsp.text)["hits"]["hits"];
    }

private:
    std::string _name;
    std::string _type;
    Json::Value _must_not;
    Json::Value _should;
    std::shared_ptr<elasticlient::Client> _client;
};
}