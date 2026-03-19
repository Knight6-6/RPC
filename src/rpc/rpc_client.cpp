#include "knight/utils/logger.hpp"
#include <cstring>
#include <knight/rpc/rpc_client.hpp>
#include <memory>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace knight::rpc
{

rpcclient::rpcclient(std::string ip , uint16_t port)
{
    naming_ip = ip;
    naming_port= port;
    udp_fd = socket(AF_INET , SOCK_DGRAM , 0);
    rpc_context_object_pool = std::make_unique<utils::objectpool<rpccontext>>(20);
    naming_context_object_pool = std::make_unique<utils::objectpool<namingcontext>>(20);
    auto client_group = std::make_unique<network::eventloopgroup>(1);
    networkclient = std::make_unique<network::client>(std::move(client_group));
    networkclient->set_tcp_task([this](uint64_t uid, char *s, size_t length)
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
            std::lock_guard<std::mutex> lock(map_lock);
            auto it=this->rpc_context_map.find(seq_id);
            if(it!=rpc_context_map.end())
            {
                rpc_object=it->second;
                rpc_context_map.erase(it);
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
    udp_fd = socket(AF_INET , SOCK_DGRAM , 0);
    networkclient->add_udp(udp_fd ,[this](const char* data , size_t length)
    {//回包格式 长度已经被拆出来了 8字节seq_id  4字节ip长度 ip  2字节端口
        std::shared_ptr<namingcontext> naming_object;
        uint64_t seq_id;
        memcpy(&seq_id , data , 8);
        seq_id = be64toh(seq_id);
        std::unique_lock<std::mutex> l(map_lock);
        {
            auto it = naming_context_map.find(seq_id);
            if(it!=naming_context_map.end())
            {
                naming_object = it->second;
                naming_context_map.erase(it);
            }
        }
        if(naming_object)
        {
            uint32_t ip_length;
            memcpy(&ip_length , data+8 , 4);
            ip_length = ntohl(ip_length);
            std::string ip(data+8+4 , ip_length);
            uint16_t port;
            memcpy(&port , data+8+4+ip_length , 2);
            port = ntohs(port);
            {
                std::unique_lock<std::mutex> l(naming_object->naming_context_lock);
                naming_object->naming_context_ip = ip;
                naming_object->naming_context_port = port;
                naming_object->naming_done = true;
            }
            naming_object->naming_context_cv.notify_one();
        }
        else utils::logger::getlogger().error(1,"获取naming_objexct 失败");
    });
}

void rpcclient::send_naming(std::string fun_name , uint64_t seq_id)
{//协议是 1字节类型 8字节seqid 4字节名字长度 名字
    std::string buf;
    buf.reserve(1+8+4+fun_name.size());
    uint8_t type = 0x02;
    buf.append(reinterpret_cast<const char*>(&type),1);
    uint64_t seq_id_ = htobe64(seq_id);
    buf.append(reinterpret_cast<const char*>(&seq_id_),8);
    uint32_t name_length =htonl(size(fun_name));
    buf.append(reinterpret_cast<const char*>(&name_length),4);
    buf.append(fun_name.data(),fun_name.size());
    sockaddr_in rpcclientaddr;
    rpcclientaddr.sin_family = AF_INET;
    rpcclientaddr.sin_port = htons(naming_port);
    inet_pton(AF_INET , naming_ip.data() , &rpcclientaddr.sin_addr);
    int fd = sendto(udp_fd , buf.data() , buf.size() , 0 , (sockaddr*)&rpcclientaddr , sizeof(rpcclientaddr));
    if(fd<0)
    {
        utils::logger::getlogger().error(1 , "rpccleint 服务发现失败");
    }
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

void namingcontext::clean()
{
    naming_context_ip.clear();
    naming_done=false;
}

}