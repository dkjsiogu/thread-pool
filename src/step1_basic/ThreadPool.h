#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

/**
 * 第一阶段：基础线程池
 * 
 * 这是最简单的线程池实现，帮助你理解核心概念：
 * 1. 工作线程池：固定数量的线程等待任务
 * 2. 任务队列：存储待执行的任务
 * 3. 同步机制：使用互斥锁和条件变量协调线程
 */
class ThreadPool {
public:
    /**
     * 构造函数：创建指定数量的工作线程
     * @param num_threads 线程池中的线程数量，默认为硬件并发数
     */
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false) {
        
        std::cout << "🚀 创建线程池，线程数量: " << num_threads << std::endl;
        
        // 创建工作线程
        for (size_t i = 0; i < num_threads; ++i) {
            // 每个线程都会执行这个 lambda 函数
            workers_.emplace_back([this, i]() {
                std::cout << "  ✓ 工作线程 #" << i << " 已启动" << std::endl;
                
                // 线程的主循环：不断从队列中取任务并执行
                while (true) {
                    std::function<void()> task;
                    
                    {
                        // 🔒 加锁：保护共享资源（任务队列和停止标志）
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        
                        // ⏰ 等待条件：
                        // 1. 有新任务到来（!tasks_.empty()）
                        // 2. 或者线程池要停止了（stop_）
                        condition_.wait(lock, [this]() {
                            return stop_ || !tasks_.empty();
                        });
                        
                        // 如果线程池停止且任务队列为空，退出循环
                        if (stop_ && tasks_.empty()) {
                            std::cout << "  ✗ 工作线程 #" << i << " 退出" << std::endl;
                            return;
                        }
                        
                        // 📦 从队列中取出一个任务
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    // 🔓 锁自动释放（离开作用域）
                    
                    // ⚙️ 执行任务（不持有锁，避免阻塞其他线程）
                    task();
                }
            });
        }
    }
    
    /**
     * 析构函数：停止所有线程并等待它们完成
     */
    ~ThreadPool() {
        std::cout << "\n🛑 正在关闭线程池..." << std::endl;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;  // 设置停止标志
        }
        
        // 唤醒所有等待的线程，让它们检查停止标志
        condition_.notify_all();
        
        // 等待所有线程完成
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "✓ 线程池已关闭" << std::endl;
    }
    
    /**
     * 提交任务到线程池
     * @param task 要执行的任务（任何可调用对象）
     * 
     * 💡 这个版本不支持返回值，任务必须是 void() 类型
     */
    void submit(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // 如果线程池已停止，不接受新任务
            if (stop_) {
                throw std::runtime_error("线程池已停止，无法提交新任务");
            }
            
            // 将任务添加到队列
            tasks_.push(std::move(task));
        }
        
        // 唤醒一个等待的线程来执行任务
        condition_.notify_one();
    }
    
    /**
     * 获取当前队列中的任务数量
     */
    size_t pending_tasks() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

private:
    // 工作线程列表
    std::vector<std::thread> workers_;
    
    // 任务队列（FIFO：先进先出）
    std::queue<std::function<void()>> tasks_;
    
    // 同步原语
    mutable std::mutex queue_mutex_;        // 保护任务队列和停止标志
    std::condition_variable condition_;      // 用于线程间通信
    
    // 停止标志
    bool stop_;
};

/**
 * 💡 关键概念解释：
 * 
 * 1. 生产者-消费者模式：
 *    - 生产者：submit() 添加任务到队列
 *    - 消费者：工作线程从队列取任务并执行
 * 
 * 2. 条件变量 (condition_variable)：
 *    - 让线程等待某个条件成立
 *    - 避免忙等待（busy waiting），节省 CPU
 * 
 * 3. 互斥锁 (mutex)：
 *    - 保护共享资源（任务队列）
 *    - 确保同一时间只有一个线程访问
 * 
 * 4. RAII (Resource Acquisition Is Initialization)：
 *    - unique_lock 自动管理锁的获取和释放
 *    - 析构函数自动清理资源
 */
