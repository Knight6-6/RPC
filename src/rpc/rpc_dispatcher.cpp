#include "knight/rpc/rpc_dispatcher.hpp"

namespace knight::rpc
{

std::string rpcdispatcher::onmessage(uint32_t fun_id, const char* data, size_t len)
{
    auto it=msg_handlers.find(fun_id);
    if(it!=msg_handlers.end())
    {
        auto msg=it->second.translator->execute(data,len);
        if(msg)
        {
            auto res=it->second.task(msg);
            if(res) return res->SerializeAsString();//将Message对象转化成string
        }
    }
    return "";
}    

void rpcdispatcher::register_service(uint32_t fun_id , google::protobuf::Service* service , const google::protobuf::MethodDescriptor* method)
{
    auto translator = std::make_shared<handlerbase>(service->GetRequestPrototype(method));//service的成员方法找到对应的method方法的工厂对象
    auto task = [service, method](std::shared_ptr<google::protobuf::Message> req) 
    {
        std::unique_ptr<google::protobuf::Message> res(service->GetResponsePrototype(method).New());
        service->CallMethod(method, nullptr, req.get(), res.get(), nullptr);//找到对应的方法并且执行
    };
    msg_handlers.try_emplace(fun_id, translator, std::move(task));
}

}
