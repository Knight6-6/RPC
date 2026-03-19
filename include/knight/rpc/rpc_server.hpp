#pragma once 
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <knight/rpc/rpc_dispatcher.hpp>
#include <knight/network/core/server.hpp>

namespace knight::rpc
{

class rpcserver
{
public:
    rpcserver(std::string namingip , uint16_t namingport);
    ~rpcserver()=default;
    void register_service(google::protobuf::Service* service);//服务端一件注入方法
    template <typename T>
    void register_handler(std::string name, std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<T>)> task)//服务端 业务方法单点注册接口
    {
        dispatcher->register_handler<T>(name , task);
    }
    bool run(std::string ip , unsigned short port);
    void send_packet(uint16_t port);
private:
    std::string naming_ip;//注册中心的ip
    std::uint16_t naming_port;//注册中心的端口;
    int udp_fd=-1;
    std::unique_ptr<rpcdispatcher> dispatcher; //rpc层对象指针
    std::unique_ptr<knight::network::server> networkserver;//网络层server指针
};

}