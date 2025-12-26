#include "io_server.hpp"
#include "message_type.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "user_manager.hpp"
#include <sys/epoll.h>
#include <cstdio>
#include <cerrno> 
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>

constexpr int IOMAX=200;

recvserver::recvserver(usermanager* user_ , objectpool<sessioncontext>* obje):user(user_),session_pool(obje){}

recvserver::~recvserver()
{
    for(auto &i:fd_to_sid)
    {
        close(i.first);
    }
    session_map.clear();
    close(epoll_fd);
}

int recvserver::EPOLL()
{
    return epoll_fd;
}

bool recvserver::add_clientInfo(int fd_ ,sockaddr_in &sockaddr)
{
    auto s=session_pool->acquire();
    s->netobject->init(sockaddr);
    uint64_t uid=session_id++;
    s->id=uid;
    s->fd=fd_;
    s->del=false;
    auto p=s.get();
    fd_to_sid.emplace(fd_,uid);
    session_map.emplace(uid,std::move(s));
    timer::gettimer().addtime(fd_ ,std::chrono::steady_clock::now()+std::chrono::seconds(30),[p](){p->del=true;});
    return true;
}

bool recvserver::init()
{
    epoll_fd=epoll_create(1);
    if(epoll_fd<0)
    {
        logger::getlogger().error(1,"epoll 初始化错误");
        return false;
    }
    return true;
}

bool recvserver::start() 
{
    while(1)
    {
        struct epoll_event events[IOMAX];
        int sum=epoll_wait(epoll_fd , events , IOMAX , 10);
        if(sum<0) logger::getlogger().error(1,"epoll_wait 出错");
        for(int i=0 ; i<sum ; i++)                      
        {
            
            int ready_fd=events[i].data.fd;
            uint32_t type_fd=events[i].events;
            auto it_fd=fd_to_sid.find(ready_fd);
            if(it_fd==fd_to_sid.end()) continue;
            auto it_se=session_map.find(it_fd->second);
            if(it_se==session_map.end()||it_se->second->del==true) continue;
            if(type_fd&EPOLLIN)
            {
                int len=it_se->second->netobject->recvhowsize_();
                if(len>0)
                {
                    char buf[1024];
                    int recv_fd=recv(ready_fd , buf , len , 0);
                    if(recv_fd<0) 
                    {
                        if(errno==EAGAIN) logger::getlogger().info("缓冲区已满，稍后重试");
                        else 
                        {
                            it_se->second->del=true;
                            logger::getlogger().error(2,"recv 错误,已断开连接");
                            continue;
                        }  
                    }
                    else if(recv_fd==0)
                    {
                        logger::getlogger().info("client %d closed gracefully", ready_fd);
                        it_se->second->del=true;
                        continue;
                    }
                    else 
                    {
                        it_se->second->netobject->recvwrite(buf , recv_fd);   
                        auto p=it_se->second.get();
                        timer::gettimer().deltime(ready_fd);
                        timer::gettimer().addtime(ready_fd ,std::chrono::steady_clock::now()+std::chrono::seconds(30),[p](){p->del=true;});
                    }
                } 
                unsigned length = 8;
                if(it_se->second->netobject->recvhowsize()>length)
                {                   
                    char buf[1024];
                    it_se->second->netobject->recvchack(buf ,length);
                    unsigned cmd_;
                    unsigned length_;
                    std::memcpy(&cmd_,buf,4);
                    std::memcpy(&length_,buf+4,4);
                    cmd_=ntohl(cmd_);
                    length_=ntohl(length_);
                    if(it_se->second->netobject->recvhowsize()>=length_)
                    {
                        CMD type=CMD(cmd_);
                        char CREATE_buf[1024];
                       it_se->second->netobject->recvread(CREATE_buf , length_);
                        char* s=CREATE_buf+length;
                        switch(type)
                        {
                            case CMD::CREATE: 
                               user->CREATEusermanager(ready_fd ,s ,it_se->second->busiobject.get());
                               break;                     
                            case CMD::LOGIN:
                               user->LOGINusermanager(ready_fd,s,it_se->second->busiobject.get());
                               break;
                            case CMD::LOGOUT:
                               user->LOGOUTusermanager(ready_fd,s,it_se->second->busiobject.get());
                               break;
                               default: break;
                        } 
                    }
                }                    
            }
            if(type_fd&EPOLLOUT)
            {
                size_t leng = it_se->second->netobject->sendhowsize();
                if(leng>0)
                {
                    char buf[1024];
                    it_se->second->netobject->sendread(buf , leng);
                    int send_fd=send(ready_fd , buf , leng , 0 );
                    if(send_fd<0) 
                    {
                        if(errno==EAGAIN)logger::getlogger().info("缓冲区满了，稍后重试");
                        else 
                        {
                           it_se->second->del=true;
                           logger::getlogger().error(2,"连接错误，已经关闭连接");
                        }
                    }
                }
            }
            if(type_fd&EPOLLRDHUP)
            {
                timer::gettimer().deltime(ready_fd);
                it_se->second->del=true;
            } 
            if(type_fd&EPOLLERR)
            {
               logger::getlogger().error(2,"连接出错");
            }
        }
        int l=user->getsize();
        if(l>0)
        {
            for(int i=0 ; i<l ; i++)
            {
                std::unique_ptr<userinformation> bu=user->begin();
                unsigned len=bu->length+8;
                int fd=bu->fd;
                auto it_fd=fd_to_sid.find(fd);
                if(it_fd==fd_to_sid.end()) continue;
                auto it_se=session_map.find(it_fd->second);
                if(it_se==session_map.end()||it_se->second->del==true) continue;
                if(len<=it_se->second->netobject->sendhowsize_())
                {
                    unsigned cmd=bu->CMD;
                    unsigned net_len;
                    unsigned net_cmd;
                    net_cmd=htonl(cmd);
                    net_len=htonl(len);
                    it_se->second->netobject->sendwrite((char*)&net_cmd,4);
                    it_se->second->netobject->sendwrite((char*)&net_len,4);
                    it_se->second->netobject->sendwrite(bu->buf.data(),len-8);
                }
            }
        }
        for(auto it=session_map.begin() ; it!=session_map.end();)
        {
            if(it->second->del)
            {
                fd_to_sid.erase(it->second->fd);
                close(it->second->fd);
                session_pool->reset(std::move(it->second));
                it=session_map.erase(it);
            }
            else it++;
        }
    }
}
