#include <endian.h>
#include <knight/rpc/rpc_server.hpp>
#include <thread>
#include <sys/socket.h>
#include <knight/utils/timer.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace knight::rpc
{
rpcserver::rpcserver(std::string ip , uint16_t port)
{
    naming_ip = ip;
    naming_port = port;
    udp_fd = socket(AF_INET , SOCK_DGRAM , 0);
    auto server_group = std::make_unique<network::eventloopgroup>(15);
    dispatcher = std::make_unique<rpcdispatcher>();
    networkserver = std::make_unique<network::server>(std::move(server_group));
    //注入回调函数
    networkserver->set_server_task([this](uint64_t uid, char *s, size_t length)
    {//包格式 4字节长度 4字节fun_id  8字节seq_id  数据
        uint32_t body_length;
        uint32_t fun_id;
        uint64_t seq_id;
        memcpy(&body_length , s , 4);
        body_length = ntohl(body_length);
        memcpy( &fun_id , s+4 , 4);
        fun_id = ntohl(fun_id);
        memcpy( &seq_id , s+8 , 8);
        seq_id = be64toh(seq_id); 
        auto res=dispatcher->onmessage(fun_id, s+16, body_length - 12);
        networkserver->send_client(uid , seq_id , res);
    });
}

bool rpcserver::run(std::string ip , unsigned short port)
{
    if(!networkserver->init(ip , port )) return false;
    std::thread server_thread([this](){networkserver->start();});
    send_packet(port);
    if(server_thread.joinable()) {
        server_thread.join();
    }
    return true;
}

void rpcserver::register_service(google::protobuf::Service* service)
{
    const google::protobuf::ServiceDescriptor* servicedes = service->GetDescriptor();//获取元信息
    std::string service_name = servicedes->name();//拿到service打包的名字
    int method_cnt = servicedes->method_count();//看service有几个方法
    for(int i=0 ; i<method_cnt ; i++)
    {
        const google::protobuf::MethodDescriptor* methoddes = servicedes->method(i);//获取每个方法的信息
        std::string full_name = service_name +'.'+methoddes->name();
        dispatcher->register_service( full_name , service , methoddes);
    }
}

void rpcserver::send_packet(uint16_t port)
{
    utils::timer::gettimer().addtime(-2 , std::chrono::steady_clock::now()+std::chrono::seconds(5) , [port , this]()
    {
        auto names = dispatcher->get_name();
        for(auto &name : names)
        {
            std::string buf;
            uint8_t type = 0x01;
            buf.append(reinterpret_cast<const char*>(&type), 1);
            uint32_t name_length = htonl(name.size());
            buf.append(reinterpret_cast<const char*>(&name_length), 4);
            buf.append(name.data(), name.size());
            uint16_t net_port = htons(port);
            buf.append(reinterpret_cast<const char*>(&net_port), 2);
            sockaddr_in rpcserveraddr;
            rpcserveraddr.sin_family = AF_INET;
            rpcserveraddr.sin_port = htons(naming_port);
            inet_pton(AF_INET , naming_ip.data() , &rpcserveraddr.sin_addr);
            sendto(udp_fd , buf.data() , buf.size() , 0 , (sockaddr*)&rpcserveraddr , sizeof(rpcserveraddr));
        }
        this->send_packet(port);
    });
}

}