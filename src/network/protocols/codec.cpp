#include "knight/network/protocols/codec.hpp"
#include "knight/utils/logger.hpp"
#include "knight/utils/timer.hpp"
#include <sys/socket.h>
#include <netinet/in.h>

#include <cerrno>

namespace knight::network
{

void codec::codecin(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it_se ,int ready_fd)
{
    int len=it_se->second->recvhowsize_();
    if(len>0)
    {
        char buf[1024];
        int recv_fd=recv(ready_fd , buf , len , 0);
        if(recv_fd<0) 
        {
            if(errno==EAGAIN) knight::utils::logger::getlogger().info("缓冲区已满，稍后重试");
            else 
            {
                it_se->second->set_del(true);
                knight::utils::logger::getlogger().error(2,"recv 错误,已断开连接");
                return;
            }  
        }
        else if(recv_fd==0)
        {
            knight::utils::logger::getlogger().info("client %d closed gracefully", ready_fd);
            it_se->second->set_del(true);
            return;
        }
        else 
        {
            it_se->second->recvwrite(buf , recv_fd);   
            auto p=it_se->second.get();
            knight::utils::timer::gettimer().deltime(ready_fd);
            knight::utils::timer::gettimer().addtime(ready_fd ,std::chrono::steady_clock::now()+std::chrono::seconds(30),[p,this](){p->set_del(true);});
        }
    } 
    unsigned length = 16;
    while(it_se->second->recvhowsize()>length)
    {                   
        char buf[1024];
        it_se->second->recvchack(buf ,length);
        uint32_t length_;
        std::memcpy(&length_,buf,4);
        length_=ntohl(length_);
        if(it_se->second->recvhowsize()>=length_)
        {
            it_se->second->recvread(buf, length_);
            it_se->second->get_fun()(it_se->first,buf+4,length_-4);
        }
    }
}

bool codecout(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it_se ,int ready_fd )
{
    size_t leng = it_se->second->sendhowsize();
    if(leng>0)
    {
        char buf[1024];
        it_se->second->sendread(buf , leng);
        int send_fd=send(ready_fd , buf , leng , 0 );
        if(send_fd<0) 
        {
            if(errno==EAGAIN)knight::utils::logger::getlogger().info("缓冲区满了，稍后重试");
            else 
            {
                it_se->second->set_del(true);
                knight::utils::logger::getlogger().error(2,"连接错误，已经关闭连接");
            }
            return false;
        }
        else return true;
    }
    return false;
}

void codecrdhup(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it_se ,int ready_fd )
{
    knight::utils::timer::gettimer().deltime(ready_fd);
    it_se->second->set_del(true);
}

}
    