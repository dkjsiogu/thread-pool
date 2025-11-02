#include "My_thread_pool.hpp"
/** 
*@param num_threads 线程池中的线程数量，默认为硬件并发数
*/
thread_pool::MyThreadPool::MyThreadPool(size_t num_threads)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back([this, i](){
            std::cout<<"start "<<i<<std::endl;
            while (true){
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock,[this](){
                        return stop_ || !tasks_.empty();  
                    });
                    
                    if (stop_ && tasks_.empty()){
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                task();
            }
        });
    }
}

thread_pool::MyThreadPool::~MyThreadPool()
{
    std::cout<<"Outing"<<std::endl;
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();  
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    std::cout<<"Outed"<<std::endl;
}

void thread_pool::MyThreadPool::submit(std::function<void()> task)
{
    {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_) {
        throw std::runtime_error("thread pool stopped!");
        }
    tasks_.push(std::move(task));
    }
    condition_.notify_all();
}

size_t thread_pool::MyThreadPool::pending_tasks() const
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}
