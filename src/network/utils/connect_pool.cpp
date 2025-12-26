#include "connect_pool.hpp"


connectpool::connectpool(){}

connectpool::~connectpool(){}

std::unique_ptr<pqxx::connection> connectpool::acquire()
{
    std::unique_ptr<pqxx::connection> se;
    std::unique_lock<std::mutex> l(lock);
    cv.wait(l,[this](){return !sqlconnect.empty()||size<sqlMAX ;});
    if(!sqlconnect.empty())
    {
       se=std::move(sqlconnect.front());
       sqlconnect.pop();
    }
    else 
    {
       std::string buf="host=127.0.0.1 port=5432 dbname=server user=knight password=874320";
       se=std::make_unique<pqxx::connection>(buf);
       size++;
    }
    return se;
}
void connectpool::reset(std::unique_ptr<pqxx::connection> se)
{
    std::lock_guard<std::mutex> l(lock);
    if(!se->is_open())
    {
       std::string buf="host=127.0.0.1 port=5432 dbname=server user=knight password=874320";
       se=std::make_unique<pqxx::connection>(buf);
    }
    sqlconnect.emplace(std::move(se));
    cv.notify_one();
}