#include <brpc/server.h>
#include <butil/logging.h>
#include "../common/etcd.hpp"
#include "../common/logger.hpp"
#include "../common/sms.hpp"
#include "../common/es_operations.hpp"
#include "../common/mysql_operations.hpp"
#include "../common/redis_operations.hpp"
#include "../common/utils.hpp"
#include "../common/channel.hpp"
#include "../proto/cpp_out/base.pb.h"
#include "../proto/cpp_out/user.pb.h"
#include "../proto/cpp_out/file.pb.h"


namespace chat_ns
{
    class UserServiceImpl : public chat_ns::UserService
    {
    public:
        UserServiceImpl(const std::shared_ptr<elasticlient::Client> &es_client,
                        const std::shared_ptr<sw::redis::Redis> &redis_client,
                        const ServiceManager::ptr &channel_manager,
                        const std::string &file_service_name)
            : _es_user(std::make_shared<ESUser>(es_client)),
              _mysql_user(std::make_shared<UserTable>()),
              _redis_session(std::make_shared<Session>(redis_client)),
              _redis_status(std::make_shared<Status>(redis_client)),
              _redis_codes(std::make_shared<Codes>(redis_client)),
              _file_service_name(file_service_name),
              _mm_channels(channel_manager)
        {
            _es_user->createIndex();
        }

    public:
        ~UserServiceImpl() {}

        static const ::google::protobuf::ServiceDescriptor *descriptor();

        void UserRegister(::google::protobuf::RpcController *controller,
                          const ::chat_ns::UserRegisterReq *request,
                          ::chat_ns::UserRegisterRsp *response,
                          ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到用户昵称注册请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            std::string nickname = request->nickname();
            std::string password = request->password();
            if (nicknameCheck(nickname) == false)
            {
                LOG_ERROR("{} - 用户名长度不合法！", request->request_id());
                return err_response(request->request_id(), "用户名长度不合法！");
            }
            if (passwordCheck(password) == false)
            {
                LOG_ERROR("{} - 密码格式不合法！", request->request_id());
                return err_response(request->request_id(), "密码格式不合法！");
            }
            User user = {0};
            if (_mysql_user->getUserByNickname(nickname, user) == true)
            {
                LOG_ERROR("{} - 用户名被占用- {}！", request->request_id(), nickname);
                return err_response(request->request_id(), "用户名被占用!");
            }
            user.user_id = Utils::uuid();
            user.nickname = nickname;
            user.password = password;
            if (_mysql_user->createUser(user) == false)
            {
                LOG_ERROR("{} - Mysql数据库新增数据失败！", request->request_id());
                return err_response(request->request_id(), "Mysql数据库新增数据失败!");
            }
            if (_es_user->appendData(user.user_id, "", user.nickname, "", "") == false)
            {
                LOG_ERROR("{} - ES搜索引擎新增数据失败！", request->request_id());
                return err_response(request->request_id(), "ES搜索引擎新增数据失败！");
            }
            response->set_request_id(request->request_id());
            response->set_success(true);
        }
        void UserLogin(::google::protobuf::RpcController *controller,
                       const ::chat_ns::UserLoginReq *request,
                       ::chat_ns::UserLoginRsp *response,
                       ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到用户昵称登录请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            std::string nickname = request->nickname();
            std::string password = request->password();
            User user = {0};
            if (_mysql_user->getUserByNickname(nickname, user) || user.password != password)
            {
                LOG_ERROR("{} - 用户名或密码错误 - {}-{}！", request->request_id(), nickname, password);
                return err_response(request->request_id(), "用户名或密码错误!");
            }
            if (_redis_status->exists(user.user_id) == true)
            {
                LOG_ERROR("{} - 用户已在其他地方登录 - {}！", request->request_id(), nickname);
                return err_response(request->request_id(), "用户已在其他地方登录!");
            }
            std::string ssid = Utils::uuid();
            _redis_session->append(ssid, user.user_id);
            _redis_status->append(user.user_id);
            response->set_request_id(request->request_id());
            response->set_login_session_id(ssid);
            response->set_success(true);
        }
        void GetPhoneVerifyCode(::google::protobuf::RpcController *controller,
                                const ::chat_ns::PhoneVerifyCodeReq *request,
                                ::chat_ns::PhoneVerifyCodeRsp *response,
                                ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到短信验证码获取请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            std::string phone = request->phone_number();
            if (phone_check(phone) == false)
            {
                LOG_ERROR("{} - 手机号码格式错误 - {}！", request->request_id(), phone);
                return err_response(request->request_id(), "手机号码格式错误!");
            }
            std::string code_id = Utils::uuid();
            std::string code = Utils::vcode();
            SMS(phone, code);
            _redis_codes->append(code_id, code);
            response->set_request_id(request->request_id());
            response->set_success(true);
            response->set_verify_code_id(code_id);
            LOG_DEBUG("获取短信验证码处理完成！");
        }
        void PhoneRegister(::google::protobuf::RpcController *controller,
                           const ::chat_ns::PhoneRegisterReq *request,
                           ::chat_ns::PhoneRegisterRsp *response,
                           ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到手机号注册请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            std::string phone = request->phone_number();
            std::string code_id = request->verify_code_id();
            std::string code = request->verify_code();
            if (phone_check(phone) == false)
            {
                LOG_ERROR("{} - 手机号码格式错误 - {}！", request->request_id(), phone);
                return err_response(request->request_id(), "手机号码格式错误!");
            }
            if (_redis_codes->code(code_id) != code)
            {
                LOG_ERROR("{} - 验证码错误 - {}-{}！", request->request_id(), code_id, code);
                return err_response(request->request_id(), "验证码错误!");
            }
            User user = {0};
            if (_mysql_user->getUserByPhone(phone, user) == true)
            {
                LOG_ERROR("{} - 该手机号已注册过用户 - {}！", request->request_id(), phone);
                return err_response(request->request_id(), "该手机号已注册过用户!");
            }
            std::string uid = Utils::uuid();
            user.user_id = uid;
            user.phone = phone;
            user.password = uid;
            user.nickname = phone;
            if (_mysql_user->createUser(user) == false)
            {
                LOG_ERROR("{} - 向数据库添加用户信息失败 - {}！", request->request_id(), phone);
                return err_response(request->request_id(), "向数据库添加用户信息失败!");
            }
            if (_es_user->appendData(uid, phone, uid, "", "") == false)
            {
                LOG_ERROR("{} - ES搜索引擎新增数据失败！", request->request_id());
                return err_response(request->request_id(), "ES搜索引擎新增数据失败！");
            }
            response->set_request_id(request->request_id());
            response->set_success(true);
        }
        void PhoneLogin(::google::protobuf::RpcController *controller,
                        const ::chat_ns::PhoneLoginReq *request,
                        ::chat_ns::PhoneLoginRsp *response,
                        ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到手机号登录请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            std::string phone = request->phone_number();
            std::string code_id = request->verify_code_id();
            std::string code = request->verify_code();
            if (phone_check(phone) == false)
            {
                LOG_ERROR("{} - 手机号码格式错误 - {}！", request->request_id(), phone);
                return err_response(request->request_id(), "手机号码格式错误!");
            }
            User user = {0};
            if (_mysql_user->getUserByPhone(phone, user) == false)
            {
                LOG_ERROR("{} - 该手机号未注册用户 - {}！", request->request_id(), phone);
                return err_response(request->request_id(), "该手机号未注册用户!");
            }
            if (_redis_codes->code(code_id) != code)
            {
                LOG_ERROR("{} - 验证码错误 - {}-{}！", request->request_id(), code_id, code);
                return err_response(request->request_id(), "验证码错误!");
            }
            _redis_codes->remove(code_id);
            if (_redis_status->exists(user.user_id) == true)
            {
                LOG_ERROR("{} - 用户已在其他地方登录 - {}！", request->request_id(), phone);
                return err_response(request->request_id(), "用户已在其他地方登录!");
            }
            std::string ssid = Utils::uuid();
            _redis_session->append(ssid, user.user_id);
            _redis_status->append(user.user_id);
            response->set_request_id(request->request_id());
            response->set_login_session_id(ssid);
            response->set_success(true);
        }
        void GetUserInfo(::google::protobuf::RpcController *controller,
                         const ::chat_ns::GetUserInfoReq *request,
                         ::chat_ns::GetUserInfoRsp *response,
                         ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到获取单个用户信息请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            // 1. 从请求中取出用户 ID
            std::string uid = request->user_id();
            // 2. 通过用户 ID，从数据库中查询用户信息
            User user = {0};
            if (_mysql_user->getUserById(uid, user) == false)
            {
                LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
                return err_response(request->request_id(), "未找到用户信息!");
            }
            // 3. 根据用户信息中的头像 ID，从文件服务器获取头像文件数据，组织完整用户信息
            UserInfo *user_info = response->mutable_user_info();
            user_info->set_user_id(user.user_id);
            user_info->set_nickname(user.nickname);
            user_info->set_description(user.description);
            user_info->set_phone(user.phone);

            if (user.avatar_id != "")
            {
                // 从信道管理对象中，获取到连接了文件管理子服务的channel
                auto channel = _mm_channels->choose(_file_service_name);
                if (!channel)
                {
                    LOG_ERROR("{} - 未找到文件管理子服务节点 - {} - {}！",
                              request->request_id(), _file_service_name, uid);
                    return err_response(request->request_id(), "未找到文件管理子服务节点!");
                }
                // 进行文件子服务的rpc请求，进行头像文件下载
                chat_ns::FileService_Stub stub(channel.get());
                chat_ns::GetSingleFileReq req;
                chat_ns::GetSingleFileRsp rsp;
                req.set_request_id(request->request_id());
                req.set_file_id(user.avatar_id);
                brpc::Controller cntl;
                stub.GetSingleFile(&cntl, &req, &rsp, nullptr);
                if (cntl.Failed() == true || rsp.success() == false)
                {
                    LOG_ERROR("{} - 文件子服务调用失败：{}！", request->request_id(), cntl.ErrorText());
                    return err_response(request->request_id(), "文件子服务调用失败!");
                }
                user_info->set_avatar(rsp.file_data().file_content());
            }
            // 4. 组织响应，返回用户信息
            response->set_request_id(request->request_id());
            response->set_success(true);
        }
        void GetMultiUserInfo(::google::protobuf::RpcController *controller,
                              const ::chat_ns::GetMultiUserInfoReq *request,
                              ::chat_ns::GetMultiUserInfoRsp *response,
                              ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到批量用户信息获取请求！");
            brpc::ClosureGuard rpc_guard(done);
            // 1. 定义错误回调
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            // 2. 从请求中取出用户ID --- 列表
            std::vector<std::string> uid_lists;
            for (int i = 0; i < request->users_id_size(); i++)
            {
                uid_lists.push_back(request->users_id(i));
            }
            // 3. 从数据库进行批量用户信息查询
            std::unordered_map<std::string, User> users;
            _mysql_user->getUsersById(uid_lists, users);
            // 4. 批量从文件管理子服务进行文件下载
            auto channel = _mm_channels->choose(_file_service_name);
            if (!channel)
            {
                LOG_ERROR("{} - 未找到文件管理子服务节点 - {}！", request->request_id(), _file_service_name);
                return err_response(request->request_id(), "未找到文件管理子服务节点!");
            }
            chat_ns::FileService_Stub stub(channel.get());
            chat_ns::GetMultiFileReq req;
            chat_ns::GetMultiFileRsp rsp;
            req.set_request_id(request->request_id());
            for (auto &[id, user] : users)
            {
                if (user.avatar_id == "")
                    continue;
                req.add_file_id_list(user.avatar_id);
            }
            brpc::Controller cntl;
            stub.GetMultiFile(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() == true || rsp.success() == false)
            {
                LOG_ERROR("{} - 文件子服务调用失败：{} - {}！", request->request_id(),
                          _file_service_name, cntl.ErrorText());
                return err_response(request->request_id(), "文件子服务调用失败!");
            }
            // 5. 组织响应（）
            for (auto &[id, user] : users)
            {
                auto user_map = response->mutable_users_info(); // 本次请求要响应的用户信息map
                auto file_map = rsp.mutable_file_data();        // 这是批量文件请求响应中的map
                UserInfo user_info;
                user_info.set_user_id(user.user_id);
                user_info.set_nickname(user.nickname);
                user_info.set_description(user.description);
                user_info.set_phone(user.phone);
                user_info.set_avatar((*file_map)[user.avatar_id].file_content());
                (*user_map)[user_info.user_id()] = user_info;
            }
            response->set_request_id(request->request_id());
            response->set_success(true);
        }
        void SetUserAvatar(::google::protobuf::RpcController *controller,
                           const ::chat_ns::SetUserAvatarReq *request,
                           ::chat_ns::SetUserAvatarRsp *response,
                           ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到用户头像设置请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            // 1. 从请求中取出用户 ID 与头像数据
            std::string uid = request->user_id();
            // 2. 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在
            User user = {0};
            if (_mysql_user->getUserById(uid, user) == false)
            {
                LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
                return err_response(request->request_id(), "未找到用户信息!");
            }
            // 3. 上传头像文件到文件子服务，
            auto channel = _mm_channels->choose(_file_service_name);
            if (!channel)
            {
                LOG_ERROR("{} - 未找到文件管理子服务节点 - {}！", request->request_id(), _file_service_name);
                return err_response(request->request_id(), "未找到文件管理子服务节点!");
            }
            chat_ns::FileService_Stub stub(channel.get());
            chat_ns::PutSingleFileReq req;
            chat_ns::PutSingleFileRsp rsp;
            req.set_request_id(request->request_id());
            req.mutable_file_data()->set_file_name("");
            req.mutable_file_data()->set_file_size(request->avatar().size());
            req.mutable_file_data()->set_file_content(request->avatar());
            brpc::Controller cntl;
            stub.PutSingleFile(&cntl, &req, &rsp, nullptr);
            if (cntl.Failed() == true || rsp.success() == false)
            {
                LOG_ERROR("{} - 文件子服务调用失败：{}！", request->request_id(), cntl.ErrorText());
                return err_response(request->request_id(), "文件子服务调用失败!");
            }
            std::string avatar_id = rsp.file_info().file_id();
            // 4. 将返回的头像文件 ID 更新到数据库中
            user.avatar_id = avatar_id;
            bool ret = _mysql_user->updateUserInfo(user);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新数据库用户头像ID失败 ：{}！", request->request_id(), avatar_id);
                return err_response(request->request_id(), "更新数据库用户头像ID失败!");
            }
            // 5. 更新 ES 服务器中用户信息
            ret = _es_user->appendData(user.user_id, user.phone,
                                       user.nickname, user.description, user.avatar_id);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新搜索引擎用户头像ID失败 ：{}！", request->request_id(), avatar_id);
                return err_response(request->request_id(), "更新搜索引擎用户头像ID失败!");
            }
            // 6. 组织响应，返回更新成功与否
            response->set_request_id(request->request_id());
            response->set_success(true);
        }
        void SetUserNickname(::google::protobuf::RpcController *controller,
                             const ::chat_ns::SetUserNicknameReq *request,
                             ::chat_ns::SetUserNicknameRsp *response,
                             ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到用户昵称设置请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            // 1. 从请求中取出用户 ID 与新的昵称
            std::string uid = request->user_id();
            std::string new_nickname = request->nickname();
            // 2. 判断昵称格式是否正确
            bool ret = nicknameCheck(new_nickname);
            if (ret == false)
            {
                LOG_ERROR("{} - 用户名长度不合法！", request->request_id());
                return err_response(request->request_id(), "用户名长度不合法！");
            }
            // 3. 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在
            User user = {0};
            if (_mysql_user->getUserById(uid, user) == false)
            {
                LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
                return err_response(request->request_id(), "未找到用户信息!");
            }
            // 4. 将新的昵称更新到数据库中
            user.nickname=new_nickname;
            ret = _mysql_user->updateUserInfo(user);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新数据库用户昵称失败 ：{}！", request->request_id(), new_nickname);
                return err_response(request->request_id(), "更新数据库用户昵称失败!");
            }
            // 5. 更新 ES 服务器中用户信息
            ret = _es_user->appendData(user.user_id, user.phone,
                                       user.nickname, user.description, user.avatar_id);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新搜索引擎用户昵称失败 ：{}！", request->request_id(), new_nickname);
                return err_response(request->request_id(), "更新搜索引擎用户昵称失败!");
            }
            // 6. 组织响应，返回更新成功与否
            response->set_request_id(request->request_id());
            response->set_success(true);
        }
        void SetUserDescription(::google::protobuf::RpcController *controller,
                                const ::chat_ns::SetUserDescriptionReq *request,
                                ::chat_ns::SetUserDescriptionRsp *response,
                                ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到用户签名设置请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            // 1. 从请求中取出用户 ID 与新的昵称
            std::string uid = request->user_id();
            std::string new_description = request->description();
            // 2. 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在

            User user = {0};
            if (_mysql_user->getUserById(uid, user) == false)
            {
                LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
                return err_response(request->request_id(), "未找到用户信息!");
            }
            // 3. 将新的签名更新到数据库中
            user.description=new_description;
            bool ret = _mysql_user->updateUserInfo(user);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新数据库用户签名失败 ：{}！", request->request_id(), new_description);
                return err_response(request->request_id(), "更新数据库用户签名失败!");
            }
            // 4. 更新 ES 服务器中用户信息
            ret = _es_user->appendData(user.user_id, user.phone,
                                       user.nickname, user.description, user.avatar_id);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新搜索引擎用户签名失败 ：{}！", request->request_id(), new_description);
                return err_response(request->request_id(), "更新搜索引擎用户签名失败!");
            }
            // 5. 组织响应，返回更新成功与否
            response->set_request_id(request->request_id());
            response->set_success(true);
        }
        void SetUserPhoneNumber(::google::protobuf::RpcController *controller,
                                const ::chat_ns::SetUserPhoneNumberReq *request,
                                ::chat_ns::SetUserPhoneNumberRsp *response,
                                ::google::protobuf::Closure *done)
        {
            LOG_DEBUG("收到用户手机号设置请求！");
            brpc::ClosureGuard rpc_guard(done);
            auto err_response = [this, response](const std::string &rid,
                                                 const std::string &errmsg) -> void
            {
                response->set_request_id(rid);
                response->set_success(false);
                response->set_errmsg(errmsg);
                return;
            };
            // 1. 从请求中取出用户 ID 与新的昵称
            std::string uid = request->user_id();
            std::string new_phone = request->phone_number();
            std::string code = request->phone_verify_code();
            std::string code_id = request->phone_verify_code_id();
            // 2. 对验证码进行验证
            auto vcode = _redis_codes->code(code_id);
            if (vcode != code)
            {
                LOG_ERROR("{} - 验证码错误 - {}-{}！", request->request_id(), code_id, code);
                return err_response(request->request_id(), "验证码错误!");
            }
            // 3. 从数据库通过用户 ID 进行用户信息查询，判断用户是否存在
            User user = {0};
            if (_mysql_user->getUserById(uid, user) == false)
            {
                LOG_ERROR("{} - 未找到用户信息 - {}！", request->request_id(), uid);
                return err_response(request->request_id(), "未找到用户信息!");
            }
            // 4. 将新的电话更新到数据库中
            user.phone=new_phone;
            bool ret = _mysql_user->updateUserInfo(user);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新数据库用户手机号失败 ：{}！", request->request_id(), new_phone);
                return err_response(request->request_id(), "更新数据库用户手机号失败!");
            }
            // 5. 更新 ES 服务器中用户信息
            ret = _es_user->appendData(user.user_id, user.phone,
                                       user.nickname, user.description, user.avatar_id);
            if (ret == false)
            {
                LOG_ERROR("{} - 更新搜索引擎用户手机号失败 ：{}！", request->request_id(), new_phone);
                return err_response(request->request_id(), "更新搜索引擎用户手机号失败!");
            }
            // 6. 组织响应，返回更新成功与否
            response->set_request_id(request->request_id());
            response->set_success(true);
        }

    private:
        bool nicknameCheck(const std::string &nickname)
        {
            return nickname.size() < 22;
        }
        bool passwordCheck(const std::string &password)
        {
            if (password.size() < 6 || password.size() > 15)
            {
                LOG_ERROR("密码长度不合法：{}-{}", password, password.size());
                return false;
            }
            for (int i = 0; i < password.size(); i++)
            {
                if (!((password[i] > 'a' && password[i] < 'z') ||
                      (password[i] > 'A' && password[i] < 'Z') ||
                      (password[i] > '0' && password[i] < '9') ||
                      password[i] == '_' || password[i] == '-'))
                {
                    LOG_ERROR("密码字符不合法：{}", password);
                    return false;
                }
            }
            return true;
        }
        bool phone_check(const std::string &phone)
        {
            if (phone.size() != 11)
                return false;
            if (phone[0] != '1')
                return false;
            if (phone[1] < '3' || phone[1] > '9')
                return false;
            for (int i = 2; i < 11; i++)
            {
                if (phone[i] < '0' || phone[i] > '9')
                    return false;
            }
            return true;
        }

    private:
        ESUser::ptr _es_user;
        UserTable::ptr _mysql_user;
        Session::ptr _redis_session;
        Status::ptr _redis_status;
        Codes::ptr _redis_codes;

        // rpc调用客户端相关对象
        std::string _file_service_name;
        ServiceManager::ptr _mm_channels;
    };

    class UserServer
    {
    public:
        using ptr = std::shared_ptr<UserServer>;
        UserServer(const Discovery::ptr service_discoverer,
                   const Registry::ptr &reg_client,
                   const std::shared_ptr<elasticlient::Client> &es_client,
                   std::shared_ptr<sw::redis::Redis> &redis_client,
                   const std::shared_ptr<brpc::Server> &server) : _service_discoverer(service_discoverer),
                                                                  _registry_client(reg_client),
                                                                  _es_client(es_client),
                                                                  _redis_client(redis_client),
                                                                  _rpc_server(server) {}
        ~UserServer() {}
        // 搭建RPC服务器，并启动服务器
        void start()
        {
            _rpc_server->RunUntilAskedToQuit();
        }

    private:
        Discovery::ptr _service_discoverer;
        Registry::ptr _registry_client;
        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<sw::redis::Redis> _redis_client;
        std::shared_ptr<brpc::Server> _rpc_server;
    };

    class UserServerBuilder
    {
    public:
        // 构造es客户端对象
        void make_es_object(const std::vector<std::string> host_list)
        {
            _es_client = ESClientFactory::create(host_list);
        }
        // 构造redis客户端对象
        void make_redis_object(const std::string &host,
                               int port,
                               int db,
                               bool keep_alive)
        {
            _redis_client = RedisClientFactory::create(host, port, db, keep_alive);
        }
        // 用于构造服务发现客户端&信道管理对象
        void make_discovery_object(const std::string &reg_host,
                                   const std::string &base_service_name,
                                   const std::string &file_service_name)
        {
            _file_service_name = file_service_name;
            _mm_channels = std::make_shared<ServiceManager>();
            _mm_channels->declared(file_service_name);
            LOG_DEBUG("设置文件子服务为需添加管理的子服务：{}", file_service_name);
            auto put_cb = std::bind(&ServiceManager::onServiceOnline, _mm_channels.get(), std::placeholders::_1, std::placeholders::_2);
            auto del_cb = std::bind(&ServiceManager::onServiceOffline, _mm_channels.get(), std::placeholders::_1, std::placeholders::_2);
            _service_discoverer = std::make_shared<Discovery>(reg_host, base_service_name, put_cb, del_cb);
        }
        // 用于构造服务注册客户端对象
        void make_registry_object(const std::string &reg_host,
                                  const std::string &service_name,
                                  const std::string &access_host)
        {
            _registry_client = std::make_shared<Registry>(reg_host);
            _registry_client->registry(service_name, access_host);
        }
        void make_rpc_server(uint16_t port, int32_t timeout, uint8_t num_threads)
        {
            if (!_es_client)
            {
                LOG_ERROR("还未初始化ES搜索引擎模块！");
                abort();
            }
            if (!_redis_client)
            {
                LOG_ERROR("还未初始化Redis数据库模块！");
                abort();
            }
            if (!_mm_channels)
            {
                LOG_ERROR("还未初始化信道管理模块！");
                abort();
            }

            _rpc_server = std::make_shared<brpc::Server>();

            UserServiceImpl *user_service = new UserServiceImpl(_es_client, _redis_client, _mm_channels, _file_service_name);
            int ret = _rpc_server->AddService(user_service,
                                              brpc::ServiceOwnership::SERVER_OWNS_SERVICE);
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
        // 构造RPC服务器对象
        UserServer::ptr build()
        {
            if (!_service_discoverer)
            {
                LOG_ERROR("还未初始化服务发现模块！");
                abort();
            }
            if (!_registry_client)
            {
                LOG_ERROR("还未初始化服务注册模块！");
                abort();
            }
            if (!_rpc_server)
            {
                LOG_ERROR("还未初始化RPC服务器模块！");
                abort();
            }
            UserServer::ptr server = std::make_shared<UserServer>(
                _service_discoverer, _registry_client,
                _es_client, _redis_client, _rpc_server);
            return server;
        }

    private:
        Registry::ptr _registry_client;

        std::shared_ptr<elasticlient::Client> _es_client;
        std::shared_ptr<sw::redis::Redis> _redis_client;
        std::string _file_service_name;
        ServiceManager::ptr _mm_channels;
        Discovery::ptr _service_discoverer;
        std::shared_ptr<brpc::Server> _rpc_server;
    };
}