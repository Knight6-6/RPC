#pragma once
#include "client.hpp"
#include "ring_buf.hpp"
#include <netinet/in.h>


class netsession
{
public:
        netsession();
        netsession(sockaddr_in& so);
        void init(sockaddr_in& so);
        void clean();
        void sendwrite(char* data, size_t len);
        void sendread(char* dest, size_t len); 
        void sendchack(char* dest ,size_t len);
        void recvwrite(char* data, size_t len);
        void recvread(char* dest, size_t len); 
        void recvchack(char* dest ,size_t len);
        size_t sendhowsize_();
        size_t recvhowsize_();
        size_t sendhowsize(); 
        size_t recvhowsize(); 
private:
        client client_;
        ringbuf sendbuf;
        ringbuf recvbuf;
};
