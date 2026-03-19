#pragma once
#include "knight/utils/object_pool.hpp"
#include "knight/network/core/event_loop.hpp"
#include "knight/network/protocols/codec.hpp"
#include <vector>
#include <atomic>
#include <functional>

namespace knight::network
{

class eventloopgroup
{
public:
    eventloopgroup(unsigned size_);
    ~eventloopgroup()=default;    
    void add_tcp_client(uint64_t uid , int fd_ , std::string ip , unsigned short port , std::function<void(uint64_t uid, char* s, size_t length)> task);
    void add_udp_client(int udp , std::function<void(const char* , size_t)> task);
    std::shared_ptr<eventloop> getio(uint64_t uid);
    uint64_t getuid();
private:
    std::vector<std::unique_ptr<knight::utils::objectpool<netsession>>> session_object;
    std::vector<std::shared_ptr<eventloop>> io_thread;
    std::unique_ptr<codec> code;
    std::atomic <uint64_t> next_io=0;
    unsigned size=0;
    std::atomic<uint64_t> session_id=0;
};

}