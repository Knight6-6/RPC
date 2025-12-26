#pragma once 
#include "object_pool.hpp"
#include "user_manager.hpp"
#include "net_session.hpp"
#include "busi_session.hpp"
#include <memory>
#include <atomic>
#include <unordered_map>

struct sessioncontext
{
    int fd;
    uint64_t id;
    std::unique_ptr<netsession> netobject;
    std::unique_ptr<busisession> busiobject;
    std::atomic<bool> del=false;
    void clean()
    {
        netobject->clean();
        busiobject->clean();
    }
    sessioncontext():netobject(std::make_unique<netsession>()),busiobject(std::make_unique<busisession>()){};
};

class recvserver
{
public:
        recvserver(usermanager*user_ , objectpool<sessioncontext>* obje);
        ~recvserver();
        bool init();
        bool start();
        int EPOLL();
        bool add_clientInfo(int fd_ ,sockaddr_in &sockaddr);
private:
        int epoll_fd;
        usermanager* user;
        objectpool<sessioncontext>* session_pool;
        std::unordered_map<uint64_t,std::unique_ptr<sessioncontext>> session_map;
        std::unordered_map<int,uint64_t> fd_to_sid;
        uint64_t session_id=1;
};
