#include <brpc/server.h>
#include <butil/logging.h>
#include "../common/asr.hpp"
#include "../common/etcd.hpp"
#include "../common/logger.hpp"
#include "speech.pb.h"
namespace chat_ns
{
    class SpeechServiceImpl : public chat_ns::SpeechService
    {
    public:
        void SpeechRecognition(google::protobuf::RpcController *controller,
                               const ::chat_ns::SpeechRecognitionReq *request,
                               ::chat_ns::SpeechRecognitionRsp *response,
                               ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到语音转文字请求！");
            brpc::ClosureGuard rpc_guard(done);
            // 1. 取出请求中的语音数据
            // 2. 调用语音sdk模块进行语音识别，得到响应

            std::string res = ASR(request->speech_content()).tostr();
            if (res.empty())
            {
                LOG_ERROR("{} 语音识别失败！", request->request_id());
                response->set_request_id(request->request_id());
                response->set_success(false);
                response->set_errmsg("语音识别失败");
                return;
            }
            // 3. 组织响应
            response->set_request_id(request->request_id());
            response->set_success(true);
            response->set_recognition_result(res);
        }
    };

    class SpeechServer
    {
    public:
        using ptr = std::shared_ptr<SpeechServer>;
        SpeechServer(
            const Registry::ptr &reg_client,
            const std::shared_ptr<brpc::Server> &server)
            : _reg_client(reg_client),
              _rpc_server(server) {}
        ~SpeechServer() {}
        // 搭建RPC服务器，并启动服务器
        void start()
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };

    class SpeechServerBuilder
    {
    public:
        // 用于构造服务注册客户端对象
        void makeRegObject(const std::string &reg_host,
                             const std::string &service_name,
                             const std::string &access_host)
        {
            _reg_client = std::make_shared<Registry>(reg_host);
            _reg_client->registry(service_name, access_host);
        }
        // 构造RPC服务器对象
        void makeRpcServer(uint16_t port, int32_t timeout, uint8_t num_threads)
        {
            _rpc_server = std::make_shared<brpc::Server>();
            SpeechServiceImpl *speech_service = new SpeechServiceImpl();
            int ret = _rpc_server->AddService(speech_service,brpc::ServiceOwnership::SERVER_OWNS_SERVICE);
            if (ret == -1)
            {
                LOG_ERROR("添加Rpc服务失败！");
                abort();
            }
            brpc::ServerOptions options;
            options.idle_timeout_sec = timeout;
            options.num_threads = num_threads;
            ret = _rpc_server->Start(port, &options);
            if (ret == -1)
            {
                LOG_ERROR("服务启动失败！");
                abort();
            }
        }
        SpeechServer::ptr build()
        {
            if (!_reg_client)
            {
                LOG_ERROR("还未初始化服务注册模块！");
                abort();
            }
            if (!_rpc_server)
            {
                LOG_ERROR("还未初始化RPC服务器模块！");
                abort();
            }
            SpeechServer::ptr server = std::make_shared<SpeechServer>(
                _reg_client, _rpc_server);
            return server;
        }

    private:
        Registry::ptr _reg_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };
}