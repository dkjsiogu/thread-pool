#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <iostream>

/**
 * 第二阶段：支持返回值的线程池
 * 
 * 新增特性：
 * 1. 使用 std::future 获取任务返回值
 * 2. 使用 std::packaged_task 包装任务
 * 3. 模板函数支持任意类型的任务
 */
class ThreadPoolWithFuture {
public:
    explicit ThreadPoolWithFuture(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false) {
        
        std::cout << "🚀 创建高级线程池，线程数量: " << num_threads << std::endl;
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i]() {
                std::cout << "  ✓ 工作线程 #" << i << " 已启动" << std::endl;
                
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this]() {
                            return stop_ || !tasks_.empty();
                        });
                        
                        if (stop_ && tasks_.empty()) {
                            std::cout << "  ✗ 工作线程 #" << i << " 退出" << std::endl;
                            return;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    // 执行任务（可能会设置 future 的值）
                    task();
                }
            });
        }
    }
    
    ~ThreadPoolWithFuture() {
        std::cout << "\n🛑 正在关闭线程池..." << std::endl;
        
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
        
        std::cout << "✓ 线程池已关闭" << std::endl;
    }
    
    /**
     * 提交任务并返回 std::future
     * 
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 要执行的函数
     * @param args 函数参数
     * @return std::future<返回值类型> 可以用来获取任务结果
     * 
     * 💡 关键技术：
     * 1. 使用模板支持任意类型的函数和参数
     * 2. std::result_of 推导函数的返回类型
     * 3. std::packaged_task 将任务和 future 绑定
     * 4. std::bind 绑定函数参数
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        // 推导返回类型
        using return_type = typename std::result_of<F(Args...)>::type;
        
        // 创建 packaged_task：将任务包装，关联一个 future
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        // 获取 future（在任务执行完成后，可以通过它获取结果）
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            if (stop_) {
                throw std::runtime_error("线程池已停止，无法提交新任务");
            }
            
            // 将任务包装成 void() 类型，放入队列
            tasks_.emplace([task]() {
                (*task)();  // 执行任务，结果会自动设置到 future 中
            });
        }
        
        condition_.notify_one();
        return result;  // 返回 future，调用者可以用它获取结果
    }
    
    size_t pending_tasks() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

/**
 * 💡 核心概念：Future 和 Promise
 * 
 * 1. std::future:
 *    - 代表一个异步操作的"未来"结果
 *    - 可以用 get() 获取结果（会阻塞直到结果可用）
 *    - 可以用 wait() 等待任务完成
 * 
 * 2. std::packaged_task:
 *    - 包装一个可调用对象
 *    - 自动创建关联的 future
 *    - 调用时会设置 future 的值
 * 
 * 3. std::bind:
 *    - 绑定函数和参数
 *    - 创建一个新的可调用对象
 * 
 * 4. 完美转发 (std::forward):
 *    - 保持参数的值类别（左值/右值）
 *    - 避免不必要的拷贝
 */
