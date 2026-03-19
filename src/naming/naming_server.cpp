#include "knight/utils/timer.hpp"
#include <cstring>
#include <knight/naming/naming_server.hpp>
#include <thread>
#include <sys/socket.h>
#include <knight/utils/logger.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace knight::naming
{

namingserver::namingserver(uint16_t port_):port(port_),table(std::make_shared<namingtable>())
{
    udp_fd = socket(AF_INET , SOCK_DGRAM , 0);
    if(udp_fd<0) 
    {
        utils::logger::getlogger().error(1,"UDP监听失败");
        return;
    }
}

void namingserver::start()
{
    clean_task();
    std::thread(&namingserver::listen_loop , this).detach();
}

void namingserver::clean_task()
{
    auto next = std::chrono::steady_clock::now()+std::chrono::seconds(10);
    utils::timer::gettimer().addtime(-1 , next , [this]()
    {
        this->table->remove_expired();
        this->clean_task();
    });
}

void namingserver::listen_loop()
{
    sockaddr_in serveraddr;
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serveraddr.sin_family= AF_INET;
    serveraddr.sin_port=htons(port);
    int bindfd = bind(udp_fd , (sockaddr*)&serveraddr , sizeof(serveraddr));
    if(bindfd<0)
    {
        utils::logger::getlogger().error(1,"naming端口绑定失败");
        return;
    }
    char buf[2048];
    sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    while(1)
    {
        size_t n = recvfrom(udp_fd, buf, sizeof(buf) , 0, (struct sockaddr*) &clientaddr,&len);
        if (n > 0) 
        {
            handle_packet(buf, n, clientaddr);
        }
    }
    close(udp_fd);
}

void namingserver::handle_packet(char* data, int length , sockaddr_in sockaddrin)
{
    const char* p = data;
    uint8_t type;
    memcpy(&type , p , 1);
    p++;
    if (type == 0x01) //0x01表示是心跳包
    { //包格式  1字节包类型  4字节名字长度  名字  2字节端口
        uint32_t name_len; 
        memcpy(&name_len , p ,4);
        name_len = ntohl(name_len);//名字长度
        p += 4;
        std::string service_name(p, name_len);//名字
        p += name_len;
        uint16_t service_port;
        memcpy(&service_port , p ,2);
        service_port = ntohs(service_port);//端口
        char buf[20];
        inet_ntop( AF_INET , &sockaddrin.sin_addr , buf , sizeof(buf));
        std::string ip(buf);
        table->register_instance(service_name, ip, service_port);
    }
    else if(type == 0x02)//服务查询包
    {//包格式  1字节类型  8字节seq_id  4字节名字长度  名字  
     //回包格式  8字节seq_id  4字节ip长度  ip  2字节端口
        std::string buf;
        buf.append(p,8);//seq_id   
        p+=8;
        uint32_t name_length; 
        memcpy(&name_length , p, 4);
        name_length = ntohl(name_length);//名字长度
        p+=4;
        std::string name( p,name_length); //名字
        auto it = table->fecth_instance(name);
        uint32_t ip_length;
        ip_length = htonl(it.ip.size()); //ip长度s
        buf.append(reinterpret_cast<const char*>(&ip_length), 4);
        buf.append(it.ip);
        uint16_t port = it.port;
        port = htons(port);
        buf.append( reinterpret_cast<const char*>(&port) , 2);
        sendto(udp_fd , buf.data() , buf.size() , 0 ,(sockaddr*)&sockaddrin , sizeof(sockaddrin));
    }
    else if(type == 0x03)//服务下线包
    {
        
    }
}

}