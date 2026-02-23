#pragma once
#include "knight/rpc/rpc_dispatcher.hpp"
#include "knight/network/core/client.hpp"
#include "knight/network/core/server.hpp"
#include "memory"
#include <condition_variable>
#include <mutex>
#include <map>
#include <knight/utils/object_pool.hpp>
#include <google/protobuf/service.h>
#include <knight/utils/logger.hpp>

namespace knight::rpc
{

class rpccontext
{
public:
    rpccontext()=default;
    void clean();
    std::mutex context_lock;
    std::condition_variable context_cv;
    std::string context_data;
    bool context_done=false;
};

class rpcprovider
{
public:
    rpcprovider();
    ~rpcprovider()=default;
    void start_blocking(std::string ip , unsigned short port);//纯服务端开发的接口
    void start_async(std::string ip , unsigned short port);//给客户端开发的接口
    void register_service(google::protobuf::Service* service);//服务端一件注入方法
    template <typename T>
    void register_handler(std::string name, std::function<std::shared_ptr<google::protobuf::Message>(std::shared_ptr<T>)> task)//服务端 业务方法单点注册接口
    {
        uint32_t fun_id = generate_methodid(name);
        dispatcher->register_handler<T>(fun_id , task);
    }
    template <typename T , typename Y>
    void invoke(std::string fun_name , T req , Y res)//客户端发起任务接口
    {
        auto fun_id = generate_methodid(fun_name);
        auto it = rpc_object_pool->acquire_shared();
        std::string data;
        //序列化
        if (!req.SerializeToString(&data)) knight::utils::logger::getlogger().error(1,"序列化失败");
        uint64_t seq_id=seq_id_get();
        {
            std::lock_guard<std::mutex> l(map_lock);
            context_map.try_emplace(seq_id , it);
        }
        networkclient->call( uid , fun_id , seq_id , data);//发送请求到对端
        std::unique_lock<std::mutex> lock(it->context_lock);
        it->context_cv.wait(lock, [it]{return it->context_done == true ;});
        if(!res.ParseFromString(it->context_data)) knight::utils::logger::getlogger().error(1,"反序列化失败");
        {
            std::lock_guard<std::mutex> l(map_lock);
            context_map.erase(seq_id);
        }
    }
    uint32_t generate_methodid(const std::string& name);//哈希值计算函数
    uint64_t seq_id_get(); 
private:
    std::atomic<uint64_t> seq_id=0;
    std::mutex map_lock;
    std::unique_ptr<rpcdispatcher> dispatcher; //rpc层对象指针
    std::unique_ptr<knight::network::client> networkclient;//网络层client指针
    std::unique_ptr<knight::network::server> networkserver;//网络层server指针
    std::map<uint64_t,std::shared_ptr<rpccontext>> context_map;//调用会话记录表
    std::unique_ptr<utils::objectpool<rpccontext>> rpc_object_pool;//对象池指针
    void run_server(std::string ip , unsigned short port);
    void run_client();
};

}