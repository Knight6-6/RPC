#pragma once 
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <vector>
#include "knight/rpc/rpc_dispatcher.hpp"
#include "knight/network/core/server.hpp"
#include "knight/naming/etcd_client.hpp"


namespace knight::rpc
{

class rpcserver
{
public:
    rpcserver(std::string url);
    ~rpcserver()=default;
    void register_service(google::protobuf::Service* service);//服务端一件注入方法
    template <typename T>
    void register_handler(std::string name, std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<T>)> task)//服务端 业务方法单点注册接口
    {
        dispatcher->register_handler<T>(name , task);
    }
    void run(std::string ip , unsigned short port);
private:
    std::unique_ptr<naming::etcd_client> etcd_client;
    std::unique_ptr<rpcdispatcher> dispatcher; //rpc层对象指针
    std::unique_ptr<knight::network::server> networkserver;//网络层server指针
    std::vector<std::string> service_names_cache;//暂存注册的service名字
};

}