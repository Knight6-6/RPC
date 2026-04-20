#include "knight/network/core/net_session.hpp"
#include <arpa/inet.h>

namespace knight::network
{

void netsession::set_ip(std::string ip)
{
   this->ip=ip;
}

void netsession::set_port(unsigned short port)
{
   this->port=port;
}

void netsession::clean()
{
   ip.clear();
   port=0;
   uid=0;
   del=false;
   sendbuf.clean();
   recvbuf.clean();
}

void netsession::sendwrite(char* data, size_t len)
{
   sendbuf.write(data,len);
}

void netsession::sendread(char* dest, size_t len)
{
   sendbuf.read(dest,len);
}

void netsession::sendchack(char* dest ,size_t len)
{
   sendbuf.chack(dest,len);
}

void netsession::recvwrite(char* data, size_t len)
{
   recvbuf.write(data,len);
}

void netsession::recvread(char* dest, size_t len)
{
   recvbuf.read(dest,len);
}

void netsession::recvchack(char* dest ,size_t len)
{
   recvbuf.chack(dest,len);
}

size_t netsession::sendhowsize_()
{
   return sendbuf.howsize_();
}

size_t netsession::recvhowsize_()
{
   return recvbuf.howsize_();
}

size_t netsession::sendhowsize()
{
   return sendbuf.howsize();
} 

size_t netsession::recvhowsize()
{
   return recvbuf.howsize();
} 

void netsession::set_id(uint64_t uid_)
{
   uid=uid_;
}
void netsession::set_fd(int fd_)
{
   fd=fd_;   
}
void netsession::set_del(bool del_)
{
   del=del_;
}
bool netsession::get_del()
{
   return del;
}

uint64_t netsession::get_uid()
{
   return uid;
}
int netsession::get_fd()
{
   return fd;
}

void netsession::set_task(std::function<void(uint64_t uid,char* s , size_t length)> fun)
{
   session_task=fun;
}

std::function<void(uint64_t uid,char* s , size_t length)>netsession::get_task()
{
   return session_task;
}

}
