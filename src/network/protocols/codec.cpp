#include "knight/network/protocols/codec.hpp"
#include "knight/utils/logger.hpp"
#include "knight/utils/timer.hpp"
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <print>

namespace knight::network
{

void codec::codecudpin(std::shared_ptr<netsession> se , int fd)
{
    char buf[1024];
    int n = recvfrom(fd, buf, sizeof(buf), 0, nullptr, nullptr);
    if (n > 0)
    {
        auto udp_task = se->get_udp_task();
        if (udp_task) 
        {
            udp_task(buf, n);
        }
        else 
        {
            knight::utils::logger::getlogger().error(2, "UDP 回调函数未注入");
        }
    }
    else if (n < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK) 
        {
            knight::utils::logger::getlogger().error(2, "UDP recvfrom 出错");
        }
    }
}

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
    unsigned head_length = 4;
    while(it_se->second->recvhowsize()>=head_length)
    {                
        char buf[4]{};
        it_se->second->recvchack(buf ,head_length);
        uint32_t body_length;
        std::memcpy(&body_length,buf,4);
        body_length=ntohl(body_length);
        if(head_length+body_length >1024 *1024)
        {
            it_se ->second->set_del(true);
            return;
        }
        if(it_se->second->recvhowsize()>=(body_length+head_length))
        {
            std::vector<char> package(head_length+body_length);      
            it_se->second->recvread(package.data(), body_length+head_length);
            it_se->second->get_task()(it_se->first,package.data(),body_length+head_length);
        }
        else break;
    }
}

bool codec::codecout(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it_se ,int ready_fd )
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

void codec::codecrdhup(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it_se ,int ready_fd )
{
    knight::utils::timer::gettimer().deltime(ready_fd);
    it_se->second->set_del(true);
}

}
    