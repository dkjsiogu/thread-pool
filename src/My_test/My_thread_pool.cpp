#include "My_thread_pool.hpp"
/** 
*@param num_threads 线程池中的线程数量，默认为硬件并发数
*/
thread_pool::MyThreadPool::MyThreadPool(size_t num_threads)
    : stop_(false)  // ✅ 在构造函数初始化列表中初始化（推荐做法）
{
    std::cout << "🚀 创建线程池，线程数量: " << num_threads << std::endl;
    
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back([this, i](){
            std::cout << "  ✓ 工作线程 #" << i << " 已启动" << std::endl;
            
            while (true){
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock,[this](){
                        return stop_ || !tasks_.empty();  
                    });
                    
                    if (stop_ && tasks_.empty()){
                        std::cout << "  ✗ 工作线程 #" << i << " 退出" << std::endl;
                        return;
                    }
                    
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                try {
                    task();
                } catch (const std::exception& e) {
                    std::cerr << "  ⚠️ 任务执行异常: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "  ⚠️ 任务执行未知异常" << std::endl;
                }
                

                active_tasks_--;
                
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    if (tasks_.empty() && active_tasks_ == 0) {
                        all_tasks_done_.notify_all();
                    }
                }
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



size_t thread_pool::MyThreadPool::pending_tasks() const
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}


void thread_pool::MyThreadPool::wait_all()
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    
    // 等待条件：队列为空 且 没有正在执行的任务
    all_tasks_done_.wait(lock, [this]() {
        return tasks_.empty() && active_tasks_ == 0;
    });
    
    std::cout << "✓ 所有任务已完成" << std::endl;
}

