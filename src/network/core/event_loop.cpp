#include "knight/network/core/event_loop.hpp"
#include "knight/utils/logger.hpp"
#include "knight/utils/timer.hpp"
#include <sys/epoll.h>
#include <cstdio>
#include <cerrno> 
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <sys/eventfd.h>

namespace knight::network
{

constexpr int IOMAX=200;

eventloop::eventloop(knight::utils::objectpool<netsession>* obje , codec* codec_):session_pool(obje),code(codec_){}

eventloop::~eventloop()
{
    for(auto &i:fd_to_uid)
    {
        close(i.first);
    }
    session_map.clear();
    close(epoll_fd);
}

int eventloop::getepoll()
{
    return epoll_fd;
}

bool eventloop::add_clientInfo(uint64_t uid_ , int fd_ , std::string ip , unsigned short port , std::function<void(uint64_t uid, char* s, size_t length)> task)
{
    struct epoll_event ev{};
    ev.events = EPOLLIN | EPOLLERR| EPOLLRDHUP;
    ev.data.fd =fd_;
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD , fd_ , &ev)<0)
    {
        knight::utils::logger::getlogger().error(1,"epoll_ctl add 错误");
    }
    auto s=session_pool->acquire_unique();
    uint64_t uid=uid_;
    s->set_ip(ip);
    s->set_port(port);
    s->set_fd(fd_);
    s->set_id(uid);
    s->set_fun(task);
    auto p=s.get(); 
    fd_to_uid.emplace(fd_,uid);
    session_map.emplace(uid,std::move(s));
    knight::utils::timer::gettimer().addtime(fd_ ,std::chrono::steady_clock::now()+std::chrono::seconds(30),[p,this](){p->set_del(true);wakeup();});
    return true;
}

void eventloop::wakeup()
{
    uint64_t one = 1;
    int wfd = write(wakeup_fd , &one , sizeof(one));
    if(wfd<0) knight::utils::logger::getlogger().error(1,"wakeup 唤醒失败");
}

void eventloop::send_data(uint64_t uid , std::string buf)
{
    {
    std::lock_guard<std::mutex> l(lock);
    push_task.push_back(std::move([this , uid , buf]() mutable
    {
        auto it = session_map.find(uid);
        if(it->second->sendhowsize()>0)
        {
            it->second->sendwrite(buf.data() , buf.size());
        } 
        else
        {
            int fd=it->second->get_fd();
            int s=send(fd , buf.data() , buf.size() , 0);
            if(s<0)
            {
                if(errno==EAGAIN)
                {
                    it->second->sendwrite(buf.data() , buf.size());
                }
            }
            else if(s<buf.size())
            {
                it->second->sendwrite(buf.data()+s , buf.size()-s);
            }
            else knight::utils::logger::getlogger().error(1,"发送失败");
        }
    }));
    }
    wakeup();
}

bool eventloop::init()
{
    epoll_fd=epoll_create(1);
    if(epoll_fd<0)
    {
        knight::utils::logger::getlogger().error(1,"epoll 初始化错误");
        return false;
    }
    wakeup_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(wakeup_fd<0)
    {
        knight::utils::logger::getlogger().error(1,"wakeup 初始化错误");
        return false;
    }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; 
    ev.data.fd = wakeup_fd;       
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, wakeup_fd, &ev) == -1)
    {
        knight::utils::logger::getlogger().error(1,"wakeup_fd 插入失败");
        return false;
    }
    return true;
}

bool eventloop::start() 
{
    while(1)
    {
        struct epoll_event events[IOMAX];
        int sum=epoll_wait(epoll_fd , events , IOMAX , 10);
        if(sum<0) knight::utils::logger::getlogger().error(1,"epoll_wait 出错");
        for(int i=0 ; i<sum ; i++)                      
        {
            int ready_fd=events[i].data.fd;
            if(ready_fd==wakeup_fd) continue;
            uint32_t type_fd=events[i].events;
            auto it_fd=fd_to_uid.find(ready_fd);
            if(it_fd==fd_to_uid.end()) continue;
            auto it_se=session_map.find(it_fd->second);
            if(it_se==session_map.end()||it_se->second->get_del()==true) continue;
            if(type_fd&EPOLLIN)
            {
                code->codecin(it_se , ready_fd);       
            }
            if(type_fd&EPOLLOUT)
            {
                code->codecout();
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLERR| EPOLLRDHUP; 
                ev.data.fd = ready_fd;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ready_fd, &ev) == -1)knight::utils::logger::getlogger().error(1, "epoll_ctl MOD (remove OUT) 出错");
            }
            if(type_fd&EPOLLRDHUP)
            {
                code->codecrdhup();
            } 
            if(type_fd&EPOLLERR)
            {
               knight::utils::logger::getlogger().error(2,"连接出错");
            }
        }
        std::vector<std::function<void()>> run_task;
        {
            std::lock_guard<std::mutex> l(lock);
            push_task.swap(run_task);
        }
        for(auto &task : run_task)
        {
            task();
        }
        for(auto it=session_map.begin() ; it!=session_map.end();)
        {
            if(it->second->get_del())
            {
                knight::utils::timer::gettimer().deltime(it->second->get_fd());
                fd_to_uid.erase(it->second->get_fd());
                close(it->second->get_fd());
                session_pool->reset(std::move(it->second));
                it=session_map.erase(it);
            }
            else it++;
        }
    }
}

}