#include "user_manager.hpp"

usermanager::usermanager(threadpool* thread_ , connectpool* connect_):thread(thread_),connect(connect_){}
usermanager::~usermanager(){}
void usermanager::CREATEusermanager(int _fd ,char *_s ,busisession* bus)
{
    std::string username(_s,32);
    std::string password(_s+32 , 32);
    thread->entask([this,_fd,username,password,bus](){
        std::unique_ptr<pqxx::connection> co = connect->acquire();   
        std::string buf;
    {
        pqxx::work txn(*co);
        pqxx::result r = txn.exec_params(
            "SELECT id FROM users WHERE username = $1 AND password = $2",
            username,
            password
        );
        if(r.empty())
        {
            txn.exec_params(
            "INSERT INTO users (username, password) VALUES ($1, $2)",
            username,
            password
           );
           buf="账号创建成功";
        }
        else buf="账号密码重复，请重试";
        txn.commit();
    } 
        connect->reset(std::move(co));
        
        bus->update(0,0);
        bus->setID(ID);
        std::lock_guard<std::mutex> l(lock);
        usersend.emplace(std::make_unique<userinformation>(_fd,buf.size(),buf,4));
    });
}
void usermanager::LOGINusermanager(int _fd ,char *_s,busisession* bus)
{
    std::string username(_s,32);
    std::string password(_s+32 , 32);
    thread->entask([this,_fd,username,password,bus](){
        std::unique_ptr<pqxx::connection> co = connect->acquire(); 
    {
        pqxx::work txn(*co);
        pqxx::result r = txn.exec_params(
            "SELECT id FROM users WHERE username = $1 AND password = $2",
            username,
            password
        );
        txn.commit();
        std::string buf;
        if(r.size())
        {
            buf="登陆成功";
            bus->update(0,1);
        }
        else buf="账号或密码错误，请重试";
        std::lock_guard<std::mutex> l(lock);
        usersend.emplace(std::make_unique<userinformation>(_fd,buf.size(),buf,5));
    }
    connect->reset(std::move(co));
    });
}
void usermanager::LOGOUTusermanager(int _fd ,char *_s ,busisession* bus)
{
    std::string username(_s,32);
    std::string password(_s+32 , 32);
    thread->entask([this,_fd,username,password,bus](){
        std::unique_ptr<pqxx::connection> co = connect->acquire(); 
    {
        pqxx::work txn(*co);
        pqxx::result r = txn.exec_params(
            "SELECT id FROM users WHERE username = $1 AND password = $2",
            username,
            password 
        );
        txn.commit();
        
        std::string buf;
        if(r.size())
        {
            buf="登出成功";
            bus->update(0,0);
        }
        else buf="账号或密码错误，请重试";
        std::lock_guard<std::mutex> l(lock);
        usersend.emplace(std::make_unique<userinformation>(_fd,buf.size(),buf,5));
    }
    connect->reset(std::move(co));
    });
}
unsigned usermanager::getsize()
{ 
    return usersend.size();
}
std::unique_ptr<userinformation> usermanager::begin()
{
    std::lock_guard<std::mutex> l(lock);
    std::unique_ptr<userinformation>information =std::move(usersend.front());
    usersend.pop();
    return information;
}