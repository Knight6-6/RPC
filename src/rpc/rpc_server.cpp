#include "knight/naming/etcd_client.hpp"
#include "knight/utils/logger.hpp"
#include <endian.h>
#include <knight/rpc/rpc_server.hpp>
#include <sys/socket.h>
#include <knight/utils/timer.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace knight::rpc
{
rpcserver::rpcserver(std::string url)
{
    etcd_client = std::make_unique<naming::etcd_client>(url);
    auto server_group = std::make_unique<network::eventloopgroup>(20);
    dispatcher = std::make_unique<rpcdispatcher>();
    networkserver = std::make_unique<network::server>(std::move(server_group));
    //注入回调函数
    networkserver->set_server_task([this](uint64_t uid, char *s, size_t length)
    {//包格式 4字节长度 4字节fun_id  8字节seq_id  数据
        uint32_t body_length;
        uint32_t fun_id;
        uint64_t seq_id;
        memcpy(&body_length , s , 4);
        body_length = ntohl(body_length);
        memcpy( &fun_id , s+4 , 4);
        fun_id = ntohl(fun_id);
        memcpy( &seq_id , s+8 , 8);
        seq_id = be64toh(seq_id); 
        auto res=dispatcher->onmessage(fun_id, s+16, body_length - 12);
        networkserver->send_client(uid , seq_id , res);
    });
}

void rpcserver::run(std::string ip , unsigned short port)
{
    if(!networkserver->init(ip , port)) return;
    for(auto i: service_names_cache)
    {
        etcd_client->register_service(i,ip,port);
    }
    knight::utils::logger::getlogger().info("所有服务已注册到 Etcd，开始监听流量...");
    service_names_cache.clear();
    networkserver->start();
}

void rpcserver::register_service(google::protobuf::Service* service)
{
    const google::protobuf::ServiceDescriptor* servicedes = service->GetDescriptor();//获取元信息
    std::string service_name = servicedes->name();//拿到service打包的名字 
    service_names_cache.push_back(service_name);
    int method_cnt = servicedes->method_count();//看service有几个方法
    for(int i=0 ; i<method_cnt ; i++)
    {
        const google::protobuf::MethodDescriptor* methoddes = servicedes->method(i);//获取每个方法的信息
        std::string full_name = service_name +'.'+methoddes->name();
        dispatcher->register_service( full_name , service , methoddes);
    }
}

}