#pragma once
#include <mutex>
#include <condition_variable>
#include <string>
#include <knight/utils/object_pool.hpp>
#include <knight/utils/logger.hpp>
#include <unordered_map>
#include <atomic>
#include <knight/network/core/client.hpp>

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

class namingcontext
{
public:
    namingcontext()=default;
    void clean();
    std::mutex naming_context_lock;
    std::condition_variable naming_context_cv;
    std::string naming_context_ip;
    std::uint16_t naming_context_port;
    bool naming_done =false;
};

class rpcclient
{
public:
    rpcclient(std::string ip , uint16_t port);
    ~rpcclient()=default;
    template <typename T , typename Y>
    void invoke(std::string fun_name , T req , Y &res)//客户端发起任务接口
    {
        auto fun_id = generate_methodid(fun_name);
        auto context_object = rpc_context_object_pool->acquire_shared();
        auto naming_object =naming_context_object_pool->acquire_shared();
        std::string data;
        //序列化
        if (!req.SerializeToString(&data)) knight::utils::logger::getlogger().error(1,"序列化失败");
        uint64_t seq_id=seq_id_get();
        uint64_t uid=0;
        {
            std::lock_guard<std::mutex> l(map_lock);
            auto ins=route_map.find(fun_name);
            if(ins!=route_map.end())
            {
                uid=ins->second;
            }  
            else naming_context_map.try_emplace(seq_id , naming_object);
            rpc_context_map.try_emplace(seq_id , context_object);
        }
        if(uid==0)
        {
            send_naming(fun_name , seq_id);//发包到注册中心
            std::unique_lock<std::mutex> lo(naming_object->naming_context_lock);
            naming_object->naming_context_cv.wait(lo , [naming_object]{return naming_object->naming_done==true ;});
            uid = networkclient->getconnect(naming_object->naming_context_ip , naming_object->naming_context_port);//uid是跟对端连接的编号
            if(uid==0) return;
            std::unique_lock<std::mutex>  lock(map_lock);
            route_map.try_emplace(fun_name , uid);
        }
        networkclient->call( uid , fun_id , seq_id , data);//发送请求到对端
        std::unique_lock<std::mutex> lock(context_object->context_lock);
        context_object->context_cv.wait(lock, [context_object]{return context_object->context_done == true ;});
        if(!res.ParseFromString(context_object->context_data)) knight::utils::logger::getlogger().error(1,"反序列化失败");
        {
            std::lock_guard<std::mutex> l(map_lock);
            rpc_context_map.erase(seq_id);
        }
    }
    uint32_t generate_methodid(const std::string& name);
    uint64_t seq_id_get(); 
    void send_naming(std::string dun_name , uint64_t seq_id);
private:
    std::atomic<uint64_t> seq_id=0;
    std::mutex map_lock;
    int udp_fd=-1;
    std::string naming_ip;//注册中心的ip
    std::uint16_t naming_port;//注册中心的端口;
    std::unique_ptr<knight::network::client> networkclient;//网络层client指针
    std::unordered_map<uint64_t,std::shared_ptr<rpccontext>> rpc_context_map;//调用会话记录表
    std::unique_ptr<utils::objectpool<rpccontext>> rpc_context_object_pool;//调用会话对象池指针
    std::unordered_map<uint64_t,std::shared_ptr<namingcontext>> naming_context_map;//注册中心通信记录表
    std::unique_ptr<utils::objectpool<namingcontext>> naming_context_object_pool;//注册中心通信对象池指针
    std::unordered_map<std::string,uint64_t>  route_map;//本地缓存fun_id到uid地映射
};

}