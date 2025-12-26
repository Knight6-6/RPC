#pragma once
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

constexpr size_t objectMAX = 500;

template <typename T>
class objectpool
{
public:
    objectpool(int size_):size(size_)
    {
        for(int i=0 ; i<size_ ; i++)
        {
            object.emplace(std::make_unique<T>());
        }
    }
    ~objectpool()=default;
    std::unique_ptr<T> acquire()
    {
        std::unique_lock<std::mutex> l(lock);
        cv.wait(l,[this](){return !object.empty()|| size<objectMAX;});
        std::unique_ptr<T> se;
        if(!object.empty())
        {
            se=std::move(object.front());
            object.pop();
        }
        else
        {
            se=std::make_unique<T>();
            size++;
        }
        l.unlock();
        return se;
    }

    void reset(std::unique_ptr<T> se)
    {
        se->clean();
        std::lock_guard<std::mutex> gl(lock);
        object.push(std::move(se));
        cv.notify_one();
    }
private:
    std::mutex lock;
    std::condition_variable cv;
    std::queue<std::unique_ptr<T>> object;
    size_t size=0;
};
