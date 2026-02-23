#include "knight/rpc/rpc_provider.hpp"
#include "knight/network/core/event_loop_group.hpp"
#include "knight/rpc/rpc_dispatcher.hpp"
#include <google/protobuf/descriptor.h>
#include <mutex>
#include <thread>


namespace knight::rpc
{

void rpccontext::clean()
{
    context_data.clear();
    context_done=false;
}

rpcprovider::rpcprovider()
{
    rpc_object_pool = std::make_unique<utils::objectpool<rpccontext>>(20);
    auto server_group = std::make_unique<network::eventloopgroup>();
    auto client_group = std::make_unique<network::eventloopgroup>();
    server_group->init(10);
    client_group->init(1);
    dispatcher = std::make_unique<rpcdispatcher>();
    networkserver = std::make_unique<network::server>(std::move(server_group));
    networkclient = std::make_unique<network::client>(std::move(client_group));
    //注入回调函数
    networkserver->set_server_task([this](uint64_t uid, char *s, size_t length)
    {
        uint32_t fun_id;
        uint64_t seq_id;
        memcpy( &fun_id , s , 4);
        memcpy( &seq_id , s+4 , 8);
        auto res=dispatcher->onmessage(fun_id, s+12, length-12);
        networkserver->send_client(uid , fun_id , seq_id , res);
    });
    networkclient->set_client_task([this](uint64_t uid, char *s, size_t length)
    {
        std::shared_ptr<rpccontext> rpc_object;
        uint64_t seq_id;
        memcpy( &seq_id , s , 8);
        {
            std::lock_guard<std::mutex> lock(map_lock);
            auto it=this->context_map.find(seq_id);
            if(it!=context_map.end())
            {
                rpc_object=it->second;
                context_map.erase(it);
            }
        }
        if(rpc_object)
        {
            {
                std::lock_guard<std::mutex> l(rpc_object->context_lock);
                rpc_object->context_data.assign(s+8,length-8);
                rpc_object->context_done=true;
            }
            rpc_object->context_cv.notify_one();
        }
        else knight::utils::logger::getlogger().error(1,"返回值回写失败");
    });
}

uint32_t rpcprovider::generate_methodid(const std::string& name) 
{
    uint32_t hash = 2166136261U; 
    for (char c : name) 
    {
        hash ^= (uint32_t)c;      
        hash *= 16777619;         
    }
    return hash;
}

void rpcprovider::run_server(std::string ip , unsigned short port)
{
    networkserver->init(ip , port);
    networkserver->start();
}

void rpcprovider::start_blocking(std::string ip , unsigned short port)
{
    run_server(ip , port);
}

void rpcprovider::start_async(std::string ip , unsigned short port)
{
    auto server_thread = std::thread([this , ip , port](){run_server(ip,port);});
    server_thread.detach();
}

void rpcprovider::register_service(google::protobuf::Service* service)// 采用protobuf的service对象一键注册
{
    const google::protobuf::ServiceDescriptor* servicedes = service->GetDescriptor();//获取元信息
    std::string service_name = servicedes->name();//拿到service打包的名字
    int method_cnt = servicedes->method_count();//看service有几个方法
    for(int i=0 ; i<method_cnt ; i++)
    {
        const google::protobuf::MethodDescriptor* methoddes = servicedes->method(i);//获取每个方法的信息
        std::string full_name = service_name +'.'+methoddes->name();
        uint32_t fun_id = generate_methodid(full_name);
        dispatcher->register_service(fun_id , service , methoddes);
    }
}

uint64_t rpcprovider::seq_id_get()
{
    return ++seq_id;
} 

}
   