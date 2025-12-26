#pragma once
#include "thread_pool.hpp"
#include "connect_pool.hpp"
#include "busi_session.hpp"
#include <string>
#include <cstring>
#include <queue>
#include <string>
#include <memory>
#include <mutex>


struct userinformation
{ 
        int fd;
        unsigned length;
        std::string buf;
        unsigned CMD;
        userinformation(int _fd , size_t _length , std::string _buf , unsigned _CMD):fd(_fd),length(_length), buf(_buf), CMD(_CMD){}
};
class usermanager
{
public:
        usermanager(threadpool* thread_ , connectpool* connect_);
        ~usermanager();
        void CREATEusermanager(int _fd ,char *_s ,busisession* bus);
        void LOGINusermanager(int _fd ,char *_s ,busisession* bus);
        void LOGOUTusermanager(int _fd,char *_s ,busisession* bus);
        unsigned getsize();
        std::unique_ptr<userinformation> begin();
private:
        std::mutex lock;
        threadpool* thread;
        connectpool* connect;
        std::queue<std::unique_ptr<userinformation>> usersend;
        int ID=0;
};  