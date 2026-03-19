#pragma once
#include "knight/rpc/rpc_handler.hpp"
#include <memory>
#include <unordered_map>
#include <google/protobuf/service.h>
#include <shared_mutex>

namespace knight::rpc
{
    
struct handlerentry
{
    handlerentry(std::shared_ptr<handlerbase> t, std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<google::protobuf::Message>)> f): translator(t), task(std::move(f)) {}
    std::shared_ptr<handlerbase> translator;
    std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<google::protobuf::Message>)> task; 
};

class rpcdispatcher 
{
public:
    rpcdispatcher()=default;
    ~rpcdispatcher()=default;
    template<typename T>//在本地map中单点插入业务方法
    void register_handler(std::string name , std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<T>)> callback)
    {
        auto translator=std::make_shared<rpchandler<T>>();
        auto task=[callback](std::shared_ptr<google::protobuf::Message> msg)
        {
            auto concrete_msg=std::static_pointer_cast<T>(msg);
            return callback(concrete_msg);  
        };
        {
            std::unique_lock<std::shared_mutex> l(map_lock);
            service_name.emplace_back(name);
        }
        {
            std::unique_lock<std::shared_mutex> l(vector_lock);
            msg_handlers.try_emplace(generate_methodid(name),translator,task);
        }
    }
    void register_service(std::string name , google::protobuf::Service* service , const google::protobuf::MethodDescriptor* method);
    //再本地map中service一键注入业务方法
    std::string onmessage(uint32_t fun_id, const char* data, size_t len);//执行具体的业务方法
    uint32_t generate_methodid(const std::string& name);
    std::vector<std::string> get_name();
    private:
    std::shared_mutex map_lock;
    std::shared_mutex vector_lock;
    std::vector<std::string> service_name;
    std::unordered_map<uint32_t , handlerentry> msg_handlers;//本地方法的注册表
};

}