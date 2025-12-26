#include "thread_pool.hpp"

threadpool::threadpool(unsigned poll)
{
    for(unsigned i=0 ; i<poll ; i++)
    {
       workthread.emplace_back([this](){this->work();});
    }
}

threadpool::~threadpool()
{
   stop=true;
   cv.notify_all();
   for(auto &i: workthread)
   {
      if(i.joinable())
      {
         i.join();
      }
   } 
}

void threadpool::work()
{
    while(!stop)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> l(lock);
            cv.wait(l,[this](){return stop || !tasks.empty();});
            if(stop)
            {
                return;
            }
            task=tasks.front();
            tasks.pop();
            task();
        }
    }
}