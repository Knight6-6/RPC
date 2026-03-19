#include "knight/rpc/rpc_dispatcher.hpp"
#include <google/protobuf/message.h>
#include <mutex>
#include <shared_mutex>

namespace knight::rpc
{

std::string rpcdispatcher::onmessage(uint32_t fun_id, const char* data, size_t len)
{
    auto it=msg_handlers.find(fun_id);
    if(it!=msg_handlers.end())
    {
        auto msg=it->second.translator->execute(data,len);//反序列化成message作为参数使用
        if(msg)
        {
            auto res=it->second.task(msg);
            if(res) return res->SerializeAsString();//将Message对象转化成protobuf流
        }
    }
    return "";
}    

void rpcdispatcher::register_service(std::string name, google::protobuf::Service* service , const google::protobuf::MethodDescriptor* method)
{
    auto translator = std::make_shared<handlerbase>(const_cast<google::protobuf::Message*>(&service->GetRequestPrototype(method)));//service的成员方法找到对应的method方法的工厂对象
    auto task = [service, method](std::shared_ptr<google::protobuf::Message> req)-> std::shared_ptr<google::protobuf::Message> 
    {
        std::shared_ptr<google::protobuf::Message> res(service->GetResponsePrototype(method).New());
        service->CallMethod(method, nullptr, req.get(), res.get(), nullptr);//找到对应的方法并且执行
        return res;
    };
    {
        std::unique_lock<std::shared_mutex> l(map_lock);
        service_name.emplace_back(name);
    }
    {
        std::unique_lock<std::shared_mutex> l(vector_lock);
        msg_handlers.try_emplace(generate_methodid(name), translator, std::move(task));
    }
}

uint32_t rpcdispatcher::generate_methodid(const std::string& name)
{
    uint32_t hash = 2166136261U; 
    for (char c : name) 
    {
        hash ^= (uint32_t)c;      
        hash *= 16777619;         
    }
    return hash;
}

std::vector<std::string> rpcdispatcher::get_name()
{
    std::shared_lock<std::shared_mutex> l(vector_lock);
    return service_name;
}

}
