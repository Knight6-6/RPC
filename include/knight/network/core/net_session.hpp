#pragma once
#include "knight/utils/ring_buf.hpp"
#include <string>
#include <functional>
#include <google/protobuf/message.h>

namespace knight::network
{

class netsession
{
public:
    netsession()=default;
    void init(std::string ip , unsigned short port);
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
    void set_id(uint64_t id_);
    void set_fd(int fd_);
    void set_del(bool del_);
    void set_ip(std::string ip);
    void set_port(unsigned short port);
    bool get_del();
    uint64_t get_uid();
    int get_fd();
    void set_udp_task(std::function<void(const char* , size_t)> task);
    void set_task(std::function<void(uint64_t fun_id,char* s , size_t length)> fun);
    std::function<void(uint64_t,char*, size_t)> get_task();
    std::function<void(const char*,size_t length)> get_udp_task();

    knight::utils::ringbuf sendbuf;
    knight::utils::ringbuf recvbuf;
    unsigned short port;
    std::string ip;
    uint64_t uid;
    int fd;
    bool del=false;
    std::function<void(const char*, size_t length)> session_udp_task;//udp的注入回调函数
    std::function<void(uint64_t uid, char* s, size_t length)> session_task;//rpc注入的传递方法，将网路层解析的数据传递给rpc层调度
};

}
