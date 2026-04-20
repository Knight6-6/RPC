#pragma once
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <string>
#include <shared_mutex>
#include "knight/utils/object_pool.hpp"
#include "knight/utils/logger.hpp"
#include <unordered_map>
#include <atomic>
#include "knight/network/core/client.hpp"
#include "knight/naming/etcd_client.hpp"                                                                

namespace knight::rpc
{

class rpccontext
{
public:
    rpccontext()=default;
    void clean();
    std::mutex context_lock;
    std::condition_variable context_cv;
    std::string context_data;//数据
    bool context_done=false;//是否触发
};

class load_balancer
{
public:
    load_balancer(int virtual_node=200);
    ~load_balancer()=default;
    void add_node(const std::string& ip_port);//添加节点
    void remove_node(const std::string& ip_port);//删除节点
    std::string select_node(const std::string& id_key);//查询节点，负载均衡获取ip
private:
    uint32_t get_hash(const std::string key);
    std::map<uint32_t,std::string> ring;//映射哈希值到物理ip
    int virtual_node;
    std::shared_mutex ring_lock; 
};

class rpcclient
{
public:
    rpcclient(std::string url);
    ~rpcclient()=default;
    template <typename T , typename Y>
    void invoke(std::string name , T req , Y &res)//客户端发起任务接口
    {
        auto fun_id = generate_methodid(name);
        auto it = name.find('.');
        auto service_name = name.substr(0,it);
        auto fun_name = name.substr(it+1);
        std::string data;
        //序列化
        if (!req.SerializeToString(&data)) 
        {
            knight::utils::logger::getlogger().error(1,"序列化失败");
            return;
        }
        int max_retries =3;//最大重试次数
        bool success = false;
        for(int i=0 ; i<max_retries ; i++)
        {
            std::string ip;
            uint16_t port;
            std::string ip_port;
            {
            std::lock_guard<std::shared_mutex> l(route_lock);
            auto load=service_route.find(service_name);
            if(load==service_route.end())
            {
                utils::logger::getlogger().error(1,"未找到该服务");
                return;
            } // 如果是重试，给 id_key 加后缀，强制哈希环指向下一个位置
            std::string retry_key = id_key + (i> 0 ? std::to_string(i) : "");
            ip_port = load->second->select_node(retry_key);
            }
            auto start = ip_port.find(':');
            ip = ip_port.substr(0,start);
            port = stoi(ip_port.substr(start+1)); 
            uint64_t uid = networkclient->get_server_connect(ip,port);
            if (uid == 0) 
            {
                knight::utils::logger::getlogger().error(1,"节点无法连接，尝试下一个...");
                continue; // 网络不通，立即换下一个节点
            }
            uint64_t seq_id = seq_id_get();
            auto context_object = context_object_pool->acquire_shared();
            {
                std::lock_guard<std::mutex> l(context_lock);
                context_map.insert({seq_id,context_object});
            }
            networkclient->call( uid , fun_id , seq_id , data);//发送请求到对端
            std::unique_lock<std::mutex> lock(context_object->context_lock);
            bool notified =context_object->context_cv.wait_for(lock,std::chrono::seconds(1), [context_object]{return context_object->context_done == true ;});
            if (notified && context_object->context_done) success=true;
            if(!res.ParseFromString(context_object->context_data)) knight::utils::logger::getlogger().error(1,"反序列化失败");
            {
                std::lock_guard<std::mutex> l(context_lock);
                context_map.erase(seq_id);
            }
            if(success) break;
        }
        if (!success)knight::utils::logger::getlogger().error(1, "RPC 调用最终失败，已尝试所有备选节点");   
    }
    uint32_t generate_methodid(const std::string& name);
    uint64_t seq_id_get(); 
    void send_naming(std::string fun_name , uint64_t seq_id);
    std::string generate_client_id();//获取机器id随机数
    std::pair<std::string, std::string> parse_etcd_key(const std::string& key);//解析出servicename跟ipport
private:
    std::string id_key;//客户端的key，用于找到对应的服务端ip
    std::unique_ptr<naming::etcd_client> etcd_client;
    std::atomic<uint64_t> seq_id=0;
    std::mutex context_lock;//会话表共用的锁
    std::shared_mutex route_lock;//路由共用的锁
    std::unique_ptr<knight::network::client> networkclient;//网络层client指针
    std::unordered_map<std::string,std::unique_ptr<load_balancer>> service_route;//映射service到哈希环
    std::unordered_map<uint64_t,std::shared_ptr<rpccontext>> context_map;//跟server调用会话记录表
    std::unique_ptr<utils::objectpool<rpccontext>> context_object_pool;//调用server会话对象池指针
};

}