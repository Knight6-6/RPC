#pragma once
#include "knight/rpc/rpc_handler.hpp"
#include <memory>
#include <unordered_map>
#include <google/protobuf/service.h>

namespace knight::rpc
{
    
struct handlerentry
{
    std::shared_ptr<handlerbase> translator;
    std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<google::protobuf::Message>)> task; 
};

class rpcdispatcher 
{
public:
    rpcdispatcher()=default;
    ~rpcdispatcher()=default;
    template<typename T>//在本地map中单点插入业务方法
    void register_handler(uint32_t fun_id, std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<T>)> callback)
    {
        auto translator=std::make_shared<rpchandler<T>>();
        auto task=[callback](std::shared_ptr<google::protobuf::Message> msg)
        {
            auto concrete_msg=std::static_pointer_cast<T>(msg);
            return callback(concrete_msg);  
        };
        msg_handlers.try_emplace(fun_id,translator,task);
    }
    void register_service(uint32_t fun_id , google::protobuf::Service* service , const google::protobuf::MethodDescriptor* method);
    //再本地map中service一键注入业务方法
    std::string onmessage(uint32_t fun_id, const char* data, size_t len);//执行具体的业务方法
    private:
    std::unordered_map<uint32_t , handlerentry> msg_handlers;//本地方法的注册表
};

}