#include "timer.hpp"
#include <sys/syscall.h>
#include <thread>

timeouttask::timeouttask(std::function<void()> task_,int fd_ ,int cycles_):task(task_),cycles(cycles_),fd(fd_){}


timer::timer()
{
    std::thread th([this](){this->work();});
    th.detach();
}

void timer::addtime(int fd,std::chrono::steady_clock::time_point timeout, std::function<void()> task)
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
        timerout[current_].emplace_back(task,fd,arrent);
        timerdel.try_emplace(fd,make_pair(current_,std::prev(timerout[current_].end())));
    }
}

void timer::deltime(int fd)
{
    auto [t,it]=timerdel[fd];
    {
        std::lock_guard<std::mutex> guard(lock);
        timerout[t].erase(it);
        timerdel.erase(fd);
    }
}

timer& timer::gettimer()
{
    static timer timerobject;
    return timerobject;
}

void timer::work()
{
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
                    tasks_to_run.push_back(it->task);   // 收集 task
                    timerdel.erase(it->fd);
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