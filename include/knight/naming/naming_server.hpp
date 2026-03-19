#pragma once
#include "knight/naming/naming_table.hpp"
#include <memory>
#include <netinet/in.h>

namespace knight::naming
{

class namingserver
{
public:
    namingserver(uint16_t port_);
    ~namingserver()=default;
    void start();//启动
private:
    void listen_loop();//开启监听
    void clean_task();//定时器开始定时清理过期任务
    void handle_packet(char* data, int length, sockaddr_in sockaddr);//解包
    int udp_fd;
    uint16_t port;
    std::shared_ptr<namingtable> table;
};

}