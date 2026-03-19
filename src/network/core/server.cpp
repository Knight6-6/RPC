#include "knight/network/core/server.hpp"
#include "knight/utils/logger.hpp"                 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

namespace knight::network
{

server::server(std::unique_ptr<eventloopgroup> group_)
{
    group=std::move(group_);
}

void server::set_server_task(std::function<void(uint64_t uid, char* s, size_t length)> server_task)
{   
    this->server_task=std::move(server_task);
}

bool server::init(std::string ip , unsigned short port)
{
    listensocket = socket(AF_INET , SOCK_STREAM , 0);
    if(listensocket<0) knight::utils::logger::getlogger().error(1,"socket创建失败");
    struct sockaddr_in serversockaddr;
    serversockaddr.sin_port=htons(port);
    serversockaddr.sin_family=AF_INET;
    inet_pton(AF_INET , ip.data() , &serversockaddr.sin_addr);
    int bind_fd=bind(listensocket , (sockaddr*)&serversockaddr , sizeof(serversockaddr));
    if(bind_fd<0) knight::utils::logger::getlogger().error(1,"绑定端口号失败");
    if(listen(listensocket ,128)<0) knight::utils::logger::getlogger().error(1,"监听失败");
    return true;
}

void server::start()
{
    while(true)
    {
        struct sockaddr_in clientbuf;
        socklen_t length=sizeof(clientbuf);
        int acceptfd=accept(listensocket ,(sockaddr*)&clientbuf , &length);
        if(acceptfd<0) 
        {
            knight::utils::logger::getlogger().error(1,"客户端连接失败");
            continue;
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientbuf.sin_addr, ip, sizeof(ip));
        unsigned short port = ntohs(clientbuf.sin_port);
        uint64_t uid = group->getuid();
        group->add_tcp_client( uid, acceptfd , ip , port , server_task);
    }
}

void server::send_client(uint64_t uid , uint64_t seq_id , std::string data)
{//回包格式  4字节长度   8字节seq_id
    std::string packet;
    auto io = group->getio(uid);
    uint32_t length = data.size() + 8;
    length = htonl(length);
    packet.append(reinterpret_cast<const char*>(&length), 4);
    seq_id = htobe64(seq_id);
    packet.append(reinterpret_cast<const char*>(&seq_id), 8);
    packet.append(reinterpret_cast<const char*>(data.data()),data.size());
    io->send_data(uid , packet);
}

}
