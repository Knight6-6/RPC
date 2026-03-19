#include "knight/utils/timer.hpp"
#include <sys/syscall.h>
#include <thread>

namespace knight::utils
{

timeouttask::timeouttask(std::function<void()> task_,int id_ ,int cycles_):task(task_),cycles(cycles_),id(id_){}

timer::timer()
{
    std::thread th([this](){this->work();});
    th.detach();
}

void timer::addtime(int id,std::chrono::steady_clock::time_point timeout, std::function<void()> task)
{
    auto now=std::chrono::steady_clock::now();
    if (timeout <= now) 
    {
        task();
        return;
    }
    auto duration=std::chrono::duration_cast<std::chrono::milliseconds>(timeout-now);
    int arrent=duration.count()/6000;
    {
        std::lock_guard<std::mutex> guard(lock);
        int current_=(duration.count()%6000/10+current)%600;
        timerout[current_].emplace_back(task,id,arrent);
        timerdel.try_emplace(id,make_pair(current_,std::prev(timerout[current_].end())));
    }
}

void timer::deltime(int id)
{
    { 
        std::lock_guard<std::mutex> guard(lock);
        auto find_it = timerdel.find(id);
        if (find_it == timerdel.end()) return; 
        auto [t,it]=timerdel[id];
        timerout[t].erase(it);
        timerdel.erase(id);
    }
}

timer& timer::gettimer()
{
    static timer timerobject;
    return timerobject;
}

void timer::work()
{
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 10000000;
    while (1)
    {
        clock_nanosleep(CLOCK_MONOTONIC, 0, &req, NULL);
        current = (current + 1) % 600;
        std::vector<std::function<void()>> tasks_to_run;
        {
            std::lock_guard<std::mutex> guard(lock);
            auto& tasks = timerout[current];
            for (auto it = tasks.begin(); it != tasks.end(); )
            {
                if (--it->cycles <= 0)
                {
                    tasks_to_run.push_back(it->task);   
                    timerdel.erase(it->id);
                    it = tasks.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        for (auto& task : tasks_to_run)
        {
            task();
        }
    }
}

}
