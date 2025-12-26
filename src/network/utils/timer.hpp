#pragma once 
#include <vector>
#include <list>
#include <functional>
#include <chrono>
#include <unordered_map>
#include <utility>
#include <time.h>
#include <mutex>

constexpr struct timespec req = { 0, 9000000 };

class timeouttask
{ 
public:
    std::function<void()> task;
    int cycles;
    int fd;
    timeouttask(std::function<void()> task_,int fd_, int cycles_);
};

class timer
{
public:
        static timer& gettimer();
        void addtime(int fd,std::chrono::steady_clock::time_point timeout, std::function<void()> task);
        void deltime(int fd);
private:
        timer();
        ~timer()=default;
        timer(timer& t)=delete;
        timer(timer&& t)=delete;
        timer operator=(timer& t)=delete;
        timer operator=(timer&& t)=delete;
        void work();
        std::mutex lock;
        size_t current=0;
        std::vector<std::list<timeouttask>> timerout{600};
        std::unordered_map<int,std::pair<int,std::list<timeouttask>::iterator>> timerdel;
};