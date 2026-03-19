#include <knight/naming/naming_table.hpp>
#include <netinet/in.h>
#include <knight/utils/timer.hpp>
#include <algorithm>

namespace knight::naming
{

instance::instance(std::string ip , unsigned short port , std::chrono::steady_clock::time_point time):ip(ip) , port(port), last_time(time){}

service::service(std::string ip , unsigned short port , std::chrono::steady_clock::time_point time):instances{{ip , port , time}},counter(0){};

void namingtable::register_instance(std::string name , std::string ip , unsigned short port)//服务注册
{
    std::unique_lock<std::shared_mutex> l(lock);
    auto it = table.find(name);
    if(it==table.end())
    {
        table.try_emplace(name, ip, port, std::chrono::steady_clock::now());
        return;
    }
    else
    {
        auto &list = it->second;
        auto it_ins = find_if(list.instances.begin() , list.instances.end() , [ip , port ](instance& ins)
        {
            return ins.ip==ip&&ins.port==port;
        });
        if(it_ins==list.instances.end())
        {
            list.instances.emplace_back(ip , port , std::chrono::steady_clock::now());
        }
        else
        {
            it_ins->last_time = std::chrono::steady_clock::now();
        }
    }
}

instance namingtable::fecth_instance(std::string name)//服务发现
{
    std::shared_lock<std::shared_mutex> l(lock);
    auto it = table.find(name);
    if(it!=table.end())
    {
        auto &list=it->second;
        auto now = std::chrono::steady_clock::now();
        uint64_t current_size = list.instances.size();
        if(current_size == 0)
        {
            return {"" , 0 , {}};
        }
        uint64_t checked_count = 0;
        uint64_t idx_ = ++list.counter;
        while(checked_count < current_size)
        {
            auto idx = (idx_ + checked_count) % current_size;
            if(now - list.instances[idx].last_time < std::chrono::seconds(15))
            {
                return list.instances[idx];
            }
            checked_count++;
        }
    } 
    return {"",0 ,{}};
}

void namingtable::remove_expired()//开启超时清除
{
    std::unique_lock<std::shared_mutex> l(lock); 
    auto now=std::chrono::steady_clock::now();
    for(auto it=table.begin() ; it!= table.end();)
    {
        auto& list=it->second;
        list.instances.erase(std::remove_if(list.instances.begin() , list.instances.end() , [now](instance &ins)
        {
            return (now - ins.last_time > std::chrono::seconds(15));
        }),list.instances.end());//remove_if把满足条件的都移到容器的末尾，返回最前面的那个迭代器
        if(list.instances.empty())
        {
            it=table.erase(it);
        }
        else it++;
    }
}

}