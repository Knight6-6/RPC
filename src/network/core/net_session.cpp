#include "net_session.hpp"

netsession::netsession(){}

netsession::netsession(sockaddr_in& so):client_(so){}

void netsession::init(sockaddr_in&  so)
{
    client_.init(so);
}

void netsession::clean()
{
    client_.clean();
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
