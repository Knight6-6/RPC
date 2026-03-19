#include "knight/network/core/event_loop_group.hpp"
#include <thread>
#include <sys/epoll.h>

namespace knight::network
{

eventloopgroup::eventloopgroup(unsigned size_)
{
    size=size_;
    code=std::make_unique<codec>();
    for(int i=0 ; i<size ; i++)
    {
        session_object.emplace_back(std::make_unique<knight::utils::objectpool<netsession>>(100));
        io_thread.emplace_back(std::make_unique<eventloop>(session_object[i].get(),code.get()));
    }      
    for(int i=0 ; i<size ; i++)
    {
       std::thread([this,i]()
       {
          io_thread[i]->start();
       }).detach();
    }   
}

 void eventloopgroup::add_udp_client(int udp , std::function<void(const char* , size_t)> task)
 {
    for(unsigned i=0 ; i< size ; i++ )
    {
        io_thread[i]->add_udp_client(udp ,task);
    }
 }

void eventloopgroup::add_tcp_client(uint64_t uid , int acceptfd , std::string ip , unsigned short port , std::function<void(uint64_t uid, char* s, size_t length)> task)
{
    getio(uid)->add_tcp_client(uid , acceptfd , ip , port , task);
}

std::shared_ptr<eventloop> eventloopgroup::getio(uint64_t uid)
{
   uint64_t io = static_cast<uint64_t>(uid >> 56);
   return io_thread[io];
}

uint64_t eventloopgroup::getuid()
{
    uint64_t io = next_io.fetch_add(1) % size;
    uint64_t current_seq = ++session_id;
    uint64_t uid = (static_cast<uint64_t>(io) << 56) | (current_seq & 0x00FFFFFFFFFFFFFF);
    return uid;
}

}
