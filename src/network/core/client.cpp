#include "knight/network/core/client.hpp"
#include <sys/socket.h>
#include "knight/utils/logger.hpp"
#include <arpa/inet.h>

namespace knight::network
{

client::client(std::unique_ptr<eventloopgroup> group_)
{
    group=std::move(group_);
}

void client::set_client_task(std::function<void(uint64_t uid, char* s, size_t length)> client_task)
{
    this->client_task=std::move(client_task);
}

uint64_t client::getconnect(std::string ip , unsigned short port)
{
    if(client_map.find({ip , port})==client_map.end())
    {
        int fd=socket(AF_INET , SOCK_STREAM , 0);
        uint64_t uid=group->getid();
        struct sockaddr_in clientconnect;
        inet_pton(AF_INET , ip.data() , &clientconnect.sin_addr);
        clientconnect.sin_port = htons(port);
        int connect_fd = connect(fd , (sockaddr*)&clientconnect , sizeof(clientconnect));
        if(connect_fd<0)
        {
            knight::utils::logger::getlogger().error(1,"连接服务器失败");
        }
        group->addclient(uid , fd , ip, port , client_task);
        client_map.emplace(std::make_pair(ip,port),uid);
    }
    uint64_t uid = client_map[std::make_pair(ip,port)];
    return uid;
}

void client::call(uint64_t uid , uint32_t fun_id , uint64_t seq_id , std::string data)
{
    std::string packet;
    uint32_t length = static_cast<uint32_t>(data.size()+12);
    packet.append(reinterpret_cast<const char*>(&length), 4);//reinterpret_cast是把这些字节当成二进制流发送过去，而不管具体的字节是表示什么 
    packet.append(reinterpret_cast<const char*>(&fun_id), 4);
    packet.append(reinterpret_cast<const char*>(&seq_id), 8);
    packet.append(reinterpret_cast<const char*>(data.data()), data.size());
    auto io=group->getio(uid);
    io->send_data(uid , packet);
}

}