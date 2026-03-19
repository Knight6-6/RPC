#pragma once
#include "knight/network/core/event_loop_group.hpp"
#include <map>
#include <utility>
#include <functional>

namespace knight::network
{

class client
{
public:
    client(std::unique_ptr<eventloopgroup> group_);
    uint64_t getconnect(std::string ip , unsigned short port);
    void call(uint64_t uid , uint32_t fun_id , uint64_t seq_id , std::string data);
    void set_tcp_task(std::function<void(uint64_t uid, char* s, size_t length)> server_task);
    void add_udp(int udp , std::function<void(const char * , size_t)> udp_task); 
private:
    std::function<void(uint64_t, char*, size_t)> tcp_task;
    std::unique_ptr<eventloopgroup> group;
    std::map<std::pair<std::string,unsigned short>,uint64_t> client_map;
};

}