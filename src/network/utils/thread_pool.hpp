#pragma once
#include <vector>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility>


class threadpool
{
public:
       threadpool(unsigned poll);
       ~threadpool();
       template<typename T>
       void entask(T&& task)
       {
           {
              std::unique_lock<std::mutex> l(lock);
              tasks.emplace(std::forward<T>(task));
           }
           cv.notify_one();
       }   
private:
       void work();
       std::queue<std::function<void()>> tasks;
       std::vector<std::thread> workthread;
       std::condition_variable cv;
       std::mutex lock;
       std::atomic<bool> stop{false};
};