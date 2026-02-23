#pragma once 
#include "knight/utils/object_pool.hpp"
#include "knight/network/protocols/codec.hpp"
#include "net_session.hpp"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace knight::network
{

class eventloop
{
public:
    eventloop(knight::utils::objectpool<netsession>*obje ,codec* code_);
    ~eventloop();
    bool init();
    bool start();
    int getepoll();
    bool add_clientInfo(uint64_t uid , int fd_ , std::string ip , unsigned short port , std::function<void(uint64_t uid, char* s, size_t length)> task);
    void send_data(uint64_t uid , std::string data);//将包发送到对端
    void wakeup();
private:
    int epoll_fd;
    int wakeup_fd;
    knight::utils::objectpool<netsession>* session_pool;
    codec* code;
    std::unordered_map<uint64_t,std::unique_ptr<netsession>> session_map;
    std::unordered_map<int,uint64_t> fd_to_uid;
    std::vector<std::function<void()>> push_task;
    std::mutex lock;
};

}
