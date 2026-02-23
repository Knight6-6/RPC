#pragma once
#include "knight/network/core/event_loop_group.hpp"
#include <cstring>
#include <functional>

namespace knight::network
{

class server
{
public:
    server(std::unique_ptr<eventloopgroup> group_);
    ~server()=default;
    bool init(std::string ip , unsigned short port);
    void start();
    void set_server_task(std::function<void(uint64_t uid, char* s, size_t length)> client_task);
    void send_client(uint64_t uid , uint32_t fun_id , uint64_t seq_id , std::string data);
private:
    std::function<void(uint64_t uid, char* s, size_t length)> server_task;
    int listensocket;
    std::unique_ptr<eventloopgroup> group;
};

}