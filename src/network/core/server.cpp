#include "server.hpp"
#include "logger.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <thread>

constexpr int IO=20; 

server::server():listensocket(-1){}

server::~server(){}

bool server::init(const char* ip , unsigned short port)
{
    listensocket = socket(AF_INET , SOCK_STREAM , 0);
    if(listensocket<0) logger::getlogger().error(1,"socket创建失败");
    struct sockaddr_in serversockaddr;
    serversockaddr.sin_port=htons(port);
    serversockaddr.sin_family=AF_INET;
    inet_pton(AF_INET , ip , &serversockaddr.sin_addr);
    int bind_fd=bind(listensocket , (sockaddr*)&serversockaddr , sizeof(serversockaddr));
    if(bind_fd<0) logger::getlogger().error(1,"绑定端口号失败");
    if(listen(listensocket ,128)<0) logger::getlogger().error(1,"监听失败");
    return true;
}
void server::start()
{
    connectpoll_=std::make_unique<connectpool>();
    threadpoll_=std::make_unique<threadpool>(20);
    sessionobject=std::make_unique<objectpool<sessioncontext>>(100);
    for(int i=0 ; i<IO ; i++)
    {
        usermanager_.emplace_back(std::make_unique<usermanager>(threadpoll_.get(), connectpoll_.get()));
        IOthread.emplace_back(std::make_unique<recvserver>(usermanager_[i].get() , sessionobject.get()));
    }      
    for(int i=0 ; i<IO ; i++)
    {
       std::thread([this,i]()
       {
          IOthread[i]->init();
          IOthread[i]->start();
       }).detach();
    }
    int nextIO=0;
    while(true)
    {
        struct sockaddr_in clientbuf;
        socklen_t length=sizeof(clientbuf);
        int acceptfd=accept(listensocket ,(sockaddr*)&clientbuf , &length);
        if(acceptfd<0) 
        {
            logger::getlogger().error(1,"客户端连接失败");
            continue;
        }
        int s= nextIO++ % IO;
        struct epoll_event ev{};
        ev.events = EPOLLIN | EPOLLERR| EPOLLOUT |EPOLLRDHUP;
        ev.data.fd =acceptfd;
        if(epoll_ctl(IOthread[s]->EPOLL(),EPOLL_CTL_ADD , acceptfd , &ev)<0)
        {
            logger::getlogger().error(1,"epoll_ctl add 错误");
        }
        IOthread[s]->add_clientInfo(acceptfd,clientbuf);
    }
}
