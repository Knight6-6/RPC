#pragma once
#include <queue>
#include <pqxx/pqxx>
#include <mutex>
#include <condition_variable>

namespace knight::utils
{

constexpr size_t sqlMAX=200;

class connectpool
{
public:
    connectpool();
    ~connectpool();
    std::unique_ptr<pqxx::connection> acquire();
    void reset(std::unique_ptr<pqxx::connection> se);
private:
    std::queue<std::unique_ptr<pqxx::connection>> sqlconnect;
    std::mutex lock;
    std::condition_variable cv;
    size_t size=0;
};

}
