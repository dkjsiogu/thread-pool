#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <atomic>
#include <iostream>
#include <chrono>

/**
 * 第三阶段：高级功能的线程池
 * 
 * 新增特性：
 * 1. 任务优先级支持
 * 2. 性能统计
 * 3. 更完善的关闭机制
 * 4. 异常安全
 */

// 任务优先级
enum class Priority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

class AdvancedThreadPool {
public:
    explicit AdvancedThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false)
        , total_tasks_(0)
        , completed_tasks_(0)
        , failed_tasks_(0) {
        
        std::cout << "🚀 创建高级线程池，线程数量: " << num_threads << std::endl;
        creation_time_ = std::chrono::steady_clock::now();
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i]() {
                std::cout << "  ✓ 工作线程 #" << i << " 已启动" << std::endl;
                
                while (true) {
                    PriorityTask task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this]() {
                            return stop_ || !tasks_.empty();
                        });
                        
                        if (stop_ && tasks_.empty()) {
                            std::cout << "  ✗ 工作线程 #" << i << " 退出" << std::endl;
                            return;
                        }
                        
                        // 从优先级队列中取出优先级最高的任务
                        task = std::move(const_cast<PriorityTask&>(tasks_.top()));
                        tasks_.pop();
                    }
                    
                    // 执行任务并统计
                    auto start = std::chrono::steady_clock::now();
                    
                    try {
                        task.func();
                        completed_tasks_++;
                    } catch (const std::exception& e) {
                        failed_tasks_++;
                        std::cerr << "  ⚠️  任务执行失败: " << e.what() << std::endl;
                    } catch (...) {
                        failed_tasks_++;
                        std::cerr << "  ⚠️  任务执行失败: 未知异常" << std::endl;
                    }
                    
                    auto end = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    
                    // 记录任务执行时间
                    {
                        std::lock_guard<std::mutex> lock(stats_mutex_);
                        total_execution_time_ += duration;
                    }
                }
            });
        }
    }
    
    ~AdvancedThreadPool() {
        shutdown();
    }
    
    /**
     * 提交任务（带优先级）
     */
    template<typename F, typename... Args>
    auto submit(Priority priority, F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            if (stop_) {
                throw std::runtime_error("线程池已停止，无法提交新任务");
            }
            
            // 将任务包装成 PriorityTask
            tasks_.emplace(priority, [task]() {
                (*task)();
            });
            
            total_tasks_++;
        }
        
        condition_.notify_one();
        return result;
    }
    
    /**
     * 提交任务（默认优先级）
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        return submit(Priority::NORMAL, std::forward<F>(f), std::forward<Args>(args)...);
    }
    
    /**
     * 优雅关闭：等待所有任务完成
     */
    void shutdown() {
        if (stop_) return;
        
        std::cout << "\n🛑 正在优雅关闭线程池..." << std::endl;
        
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
        
        print_statistics();
        std::cout << "✓ 线程池已关闭" << std::endl;
    }
    
    /**
     * 强制关闭：丢弃未执行的任务
     */
    void shutdown_now() {
        std::cout << "\n⚠️  强制关闭线程池..." << std::endl;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // 清空任务队列
            size_t discarded = tasks_.size();
            while (!tasks_.empty()) {
                tasks_.pop();
            }
            
            std::cout << "  丢弃了 " << discarded << " 个未执行的任务" << std::endl;
            stop_ = true;
        }
        
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        print_statistics();
        std::cout << "✓ 线程池已强制关闭" << std::endl;
    }
    
    /**
     * 获取统计信息
     */
    void print_statistics() const {
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            now - creation_time_).count();
        
        std::cout << "\n📊 线程池统计信息：" << std::endl;
        std::cout << "  运行时间: " << uptime << " 秒" << std::endl;
        std::cout << "  总任务数: " << total_tasks_.load() << std::endl;
        std::cout << "  已完成: " << completed_tasks_.load() << std::endl;
        std::cout << "  失败: " << failed_tasks_.load() << std::endl;
        std::cout << "  待处理: " << pending_tasks() << std::endl;
        
        if (completed_tasks_ > 0) {
            double avg_time = (double)total_execution_time_ / completed_tasks_;
            std::cout << "  平均执行时间: " << avg_time << " 微秒" << std::endl;
        }
    }
    
    size_t pending_tasks() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }
    
    size_t thread_count() const {
        return workers_.size();
    }
    
    size_t total_tasks() const {
        return total_tasks_.load();
    }
    
    size_t completed_tasks() const {
        return completed_tasks_.load();
    }

private:
    // 优先级任务包装
    struct PriorityTask {
        Priority priority;
        std::function<void()> func;
        
        PriorityTask(Priority p = Priority::NORMAL, std::function<void()> f = nullptr)
            : priority(p), func(std::move(f)) {}
        
        // 优先级比较（用于优先级队列）
        bool operator<(const PriorityTask& other) const {
            return priority < other.priority;
        }
    };
    
    std::vector<std::thread> workers_;
    std::priority_queue<PriorityTask> tasks_;  // 优先级队列
    
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
    
    // 统计信息
    std::atomic<size_t> total_tasks_;
    std::atomic<size_t> completed_tasks_;
    std::atomic<size_t> failed_tasks_;
    std::chrono::steady_clock::time_point creation_time_;
    
    mutable std::mutex stats_mutex_;
    long long total_execution_time_ = 0;  // 微秒
};

/**
 * 💡 高级特性说明：
 * 
 * 1. 优先级队列：
 *    - 使用 std::priority_queue 替代 std::queue
 *    - 高优先级任务会先执行
 * 
 * 2. 性能统计：
 *    - 使用 std::atomic 统计任务数量（线程安全）
 *    - 记录任务执行时间
 *    - 提供统计报告
 * 
 * 3. 关闭机制：
 *    - shutdown(): 优雅关闭，等待所有任务完成
 *    - shutdown_now(): 强制关闭，丢弃未执行的任务
 * 
 * 4. 异常安全：
 *    - 捕获任务中的所有异常
 *    - 不会因为单个任务失败而崩溃
 *    - 记录失败的任务数量
 */
