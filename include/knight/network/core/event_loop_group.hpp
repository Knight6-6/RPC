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
    eventloopgroup()=default;
    ~eventloopgroup()=default;
    void init(unsigned size);     
    void addclient(uint64_t uid , int fd_ , std::string ip , unsigned short port , std::function<void(uint64_t uid, char* s, size_t length)> task);
    std::shared_ptr<eventloop> getio(uint64_t uid);
    uint64_t getid();
private:
    std::vector<std::unique_ptr<knight::utils::objectpool<netsession>>> sessionobject;
    std::vector<std::shared_ptr<eventloop>> io_thread;
    std::unique_ptr<codec> code;
    unsigned next_io=1;
    unsigned size=0;
    std::atomic<uint64_t> session_id=0;
};

}