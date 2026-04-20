#include "knight/naming/etcd_client.hpp"
#include "knight/utils/logger.hpp"
#include <cstring>
#include <knight/rpc/rpc_client.hpp>
#include <memory>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <random>

namespace knight::rpc
{
load_balancer::load_balancer(int virtual_node):virtual_node(virtual_node){}

uint32_t load_balancer::get_hash(const std::string key)
{
    // 使用 FNV-1a 算法
    uint32_t hash = 2166136261U;
    for (char c : key) 
    {
        hash ^= static_cast<uint8_t>(c);
        hash *= 16777619U;
    }
    return hash;
}
void load_balancer::add_node(const std::string& ip_port)
{
    std::unique_lock lock(ring_lock);
    for (int i = 0; i < virtual_node; ++i) 
    {
        // 串联逻辑：IP + # + 序号 -> 算出哈希值
        uint32_t v_hash = get_hash(ip_port + "#" + std::to_string(i));
        ring[v_hash] = ip_port;
    }
    
}
void load_balancer::remove_node(const std::string& ip_port)
{
    std::unique_lock lock(ring_lock);
    std::erase_if(ring,[&](const auto& it)
    {
        return it.second == ip_port;
    });
}
std::string load_balancer::select_node(const std::string& id_key)
{
    std::shared_lock lock(ring_lock);
    uint32_t client_hash =get_hash(id_key);
    auto it =ring.lower_bound(client_hash);
    if(it==ring.end()) it =ring.begin();
    return it->second;
}

std::pair<std::string, std::string> rpcclient::parse_etcd_key(const std::string& key) 
{
    // 假设格式为: /service/ServiceName/IP:Port
    // 找最后两个斜杠
    size_t last_slash = key.find_last_of('/');
    if (last_slash == std::string::npos) return {"", ""};
    size_t second_last_slash = key.find_last_of('/', last_slash - 1);
    if (second_last_slash == std::string::npos) return {"", ""};
    std::string service_name = key.substr(second_last_slash + 1, last_slash - second_last_slash - 1);
    std::string ip_port = key.substr(last_slash + 1);
    return {service_name, ip_port};
}

rpcclient::rpcclient(std::string url)
{
    id_key = generate_client_id();
    etcd_client = std::make_unique<naming::etcd_client>(url);
    context_object_pool = std::make_unique<utils::objectpool<rpccontext>>(20);
    auto client_group = std::make_unique<network::eventloopgroup>(1);
    networkclient = std::make_unique<network::client>(std::move(client_group));

    auto services_route = etcd_client->get_all_services();
    for(auto [service,ip_port] : services_route)
    {
        std::unique_lock<std::shared_mutex> lock(this->route_lock);
        if(service_route.find(service)==service_route.end())
        {
            service_route[service] = std::make_unique<load_balancer>(200);
        }
        for(auto i:ip_port) service_route[service]->add_node(i);
    }//初始化，将注册中心的所有信息加载到本地哈希环
    etcd_client->watch_prefix("/service/", [this](etcd::Response resp)
    {
        for (auto const& event : resp.events()) 
        {
            std::string key = event.kv().key();     // 例如: "/service/UserService/192.168.1.10:8001"
            std::string value = event.kv().as_string(); // 例如: "192.168.1.10:8001"
            // 2. 解析 Key，提取服务名和 IP:Port
            auto [service_name, ip_port] = this->parse_etcd_key(key);
            // 如果解析失败（比如路径格式不对），直接跳过
            if (service_name.empty() || ip_port.empty()) continue;
            // 3. 找到对应的服务哈希环
            {
                std::unique_lock<std::shared_mutex> lock(this->route_lock);
                auto it = this->service_route.find(service_name);
                if (it == this->service_route.end()) service_route[service_name] = std::make_unique<load_balancer>(200);
                auto& lb = it->second; // 拿到对应的 load_balancer 指针
                // 4. 根据事件类型执行对应操作
                if (event.event_type() == etcd::Event::EventType::PUT) 
                {
                    // 新节点上线 或 租约续期（add_node 内部通常要处理幂等）
                    lb->add_node(ip_port);
                } 
                else if (event.event_type() == etcd::Event::EventType::DELETE_) 
                {
                    // 节点下线（租约到期或手动删除）
                    lb->remove_node(ip_port);
                }
            }
        }
    });//注入watch触发回调函数
    networkclient->set_server_task([this](uint64_t uid, char *s, size_t length)
    {
        //回包格式 4字节长度 8字节seq_id 剩下的都是数据
        std::shared_ptr<rpccontext> rpc_object;
        uint32_t body_length;
        uint64_t seq_id;
        memcpy( &body_length, s , 4);
        body_length = ntohl(body_length);
        memcpy( &seq_id , s+4 , 8);
        seq_id = be64toh(seq_id);
        {
            std::lock_guard<std::mutex> lock(context_lock);
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
                rpc_object->context_data.assign(s+12,body_length-8);
                rpc_object->context_done=true;
            }
            rpc_object->context_cv.notify_one();
        }
        else knight::utils::logger::getlogger().error(1,"返回值回写失败");
    });
}

uint32_t rpcclient::generate_methodid(const std::string& name)
{
    uint32_t hash = 2166136261U; 
    for (char c : name) 
    {
        hash ^= (uint32_t)c;      
        hash *= 16777619;         
    }
    return hash;
}

uint64_t rpcclient::seq_id_get()
{
    return ++seq_id;
} 

void rpccontext::clean()
{
    context_data.clear();
    context_done=false;
}

std::string rpcclient::generate_client_id() 
{
    // 1. 获取真随机数种子（由操作系统提供）
    std::random_device rd; 
    // 2. 初始化引擎（梅森旋转算法，目前最通用的算法）
    std::mt19937_64 gen(rd()); 
    // 3. 定义数字范围（产生一个从 0 到 最大无符号 64 位整数之间的数）
    std::uniform_int_distribution<unsigned long long> dis;
    // 4. 生成数字并转成字符串
    return std::to_string(dis(gen));
}

}