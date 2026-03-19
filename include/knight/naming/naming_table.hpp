#pragma once
#include <chrono>
#include <google/protobuf/descriptor.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <shared_mutex>
#include <atomic>

namespace knight::naming
{

class instance
{
public:
    instance(std::string ip , unsigned short port , std::chrono::steady_clock::time_point time);   
    std::string ip;
    unsigned short port;
    std::chrono::steady_clock::time_point last_time;
};

class service
{
public:
    service(std::string ip , unsigned short port , std::chrono::steady_clock::time_point time);
    std::vector<instance> instances;
    std::atomic<uint64_t> counter;
};

class namingtable
{
public:
    namingtable()=default;
    ~namingtable()=default;
    void register_instance(std::string name , std::string ip , unsigned short port);//服务注册
    instance fecth_instance(std::string name);//服务发现
    void remove_expired();//超时服务删除接口
    private:
    std::unordered_map<std::string,service> table;//服务记录表
    std::shared_mutex lock;
};

}