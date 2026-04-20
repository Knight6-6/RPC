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
    uint64_t get_server_connect(std::string ip , unsigned short port);
    void call(uint64_t uid , uint32_t fun_id , uint64_t seq_id , std::string data);//发包到server
    void set_naming_task(std::function<void(uint64_t uid, char* s, size_t length)> naming_task);
    void set_connect_maming(uint64_t naming_uid , std::function<void()> task);
    void set_server_task(std::function<void(uint64_t uid, char* s, size_t length)> server_task);
private:
    std::function<void(uint64_t, char*, size_t)> server_task;
    std::unique_ptr<eventloopgroup> group;
    std::map<std::pair<std::string,unsigned short>,uint64_t> client_map;
};

}