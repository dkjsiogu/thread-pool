#include "My_thread_pool.hpp"
thread_pool::MyThreadPool::MyThreadPool(size_t num_threads)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back([this, i](){
            std::cout<<"start "<<i<<std::endl;
            while (true){
                std::function<void()> task;{
                    
                    {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock,[this](){
                        return stop_||tasks_.empty();
                        });
                    
                    }

                    
                    
                    }
                }
            });
    }
}
