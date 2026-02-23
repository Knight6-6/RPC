#include "knight/network/core/event_loop_group.hpp"
#include <thread>
#include <sys/epoll.h>

namespace knight::network
{

void eventloopgroup::init(unsigned size_)
{
    size=size_;
    code=std::make_unique<codec>();
    for(int i=0 ; i<size ; i++)
    {
        sessionobject.emplace_back(std::make_unique<knight::utils::objectpool<netsession>>(100));
        io_thread.emplace_back(std::make_unique<eventloop>(sessionobject[i].get(),code.get()));
    }      
    for(int i=0 ; i<size ; i++)
    {
       std::thread([this,i]()
       {
          io_thread[i]->init();
          io_thread[i]->start();
       }).detach();
    }   
}

void eventloopgroup::addclient(uint64_t uid , int acceptfd , std::string ip , unsigned short port , std::function<void(uint64_t uid, char* s, size_t length)> task)
{
    io_thread[next_io]->add_clientInfo(uid , acceptfd , ip , port , task);
    next_io= next_io++%size;
}

std::shared_ptr<eventloop> eventloopgroup::getio(uint64_t uid)
{
   uint64_t io = static_cast<uint64_t>(uid >> 56);
   return io_thread[io];
}

uint64_t eventloopgroup::getid()
{
    uint64_t current_seq = ++session_id;
    uint64_t uid = (static_cast<uint64_t>(next_io) << 56) | (current_seq & 0x00FFFFFFFFFFFFFF);
    return uid;
}

}
