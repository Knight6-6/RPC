#pragma once
#include "thread_pool.hpp"
#include "user_manager.hpp"
#include "io_server.hpp"
#include "object_pool.hpp"
#include "connect_pool.hpp"
#include <cstring>
#include <vector>


class server
{
public:
        server();
        ~server();
        bool init(const char* ip , unsigned short port);
        void start();
private:
        int listensocket;
        std::unique_ptr<threadpool> threadpoll_;
        std::unique_ptr<objectpool<sessioncontext>> sessionobject;
        std::unique_ptr<connectpool> connectpoll_;
        std::vector<std::unique_ptr<usermanager>> usermanager_;
        std::vector<std::unique_ptr<recvserver>> IOthread;
};