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
#include <chrono>
#include <stdexcept>

/**
 * 🎯 生产级线程池 - 最终版本
 * 
 * 特性：
 * ✅ 支持任务返回值 (std::future)
 * ✅ 任务优先级
 * ✅ 性能统计
 * ✅ 异常安全
 * ✅ 优雅关闭
 * ✅ 线程安全
 * ✅ 易于使用的 API
 */

namespace thread_pool {

enum class Priority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

class ThreadPool {
public:
    /**
     * 构造函数
     * @param num_threads 线程数量，默认为硬件并发数
     * @param enable_stats 是否启用统计功能，默认启用
     */
    explicit ThreadPool(
        size_t num_threads = std::thread::hardware_concurrency(),
        bool enable_stats = true)
        : stop_(false)
        , enable_stats_(enable_stats)
        , total_tasks_(0)
        , completed_tasks_(0)
        , failed_tasks_(0)
        , total_execution_time_(0) {
        
        if (num_threads == 0) {
            throw std::invalid_argument("线程数量必须大于0");
        }
        
        creation_time_ = std::chrono::steady_clock::now();
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() {
                worker_thread();
            });
        }
    }
    
    /**
     * 析构函数 - 自动调用 shutdown()
     */
    ~ThreadPool() {
        shutdown();
    }
    
    // 禁止拷贝和移动
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    
    /**
     * 提交任务（带优先级）
     * 
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param priority 任务优先级
     * @param f 要执行的函数
     * @param args 函数参数
     * @return std::future<返回值类型>
     * 
     * 示例:
     *   auto result = pool.submit(Priority::HIGH, [](int x) { return x * 2; }, 21);
     *   std::cout << result.get() << std::endl;  // 输出 42
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
                throw std::runtime_error("Cannot submit task: ThreadPool is stopped");
            }
            
            tasks_.emplace(priority, [this, task]() {
                execute_task([task]() { (*task)(); });
            });
            
            if (enable_stats_) {
                total_tasks_++;
            }
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
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) return;  // 已经关闭
            stop_ = true;
        }
        
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    /**
     * 强制关闭：丢弃未执行的任务
     */
    void shutdown_now() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) return;
            
            // 清空任务队列
            while (!tasks_.empty()) {
                tasks_.pop();
            }
            
            stop_ = true;
        }
        
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    /**
     * 查询接口
     */
    size_t thread_count() const { return workers_.size(); }
    size_t pending_tasks() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }
    size_t total_tasks() const { return total_tasks_.load(); }
    size_t completed_tasks() const { return completed_tasks_.load(); }
    size_t failed_tasks() const { return failed_tasks_.load(); }
    bool is_stopped() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return stop_;
    }
    
    /**
     * 获取平均任务执行时间（微秒）
     */
    double average_execution_time() const {
        if (!enable_stats_ || completed_tasks_ == 0) {
            return 0.0;
        }
        return static_cast<double>(total_execution_time_) / completed_tasks_;
    }
    
    /**
     * 获取运行时间（秒）
     */
    long long uptime() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(
            now - creation_time_).count();
    }

private:
    /**
     * 优先级任务包装
     */
    struct PriorityTask {
        Priority priority;
        std::function<void()> func;
        
        PriorityTask(Priority p = Priority::NORMAL, std::function<void()> f = nullptr)
            : priority(p), func(std::move(f)) {}
        
        bool operator<(const PriorityTask& other) const {
            return priority < other.priority;
        }
    };
    
    /**
     * 工作线程函数
     */
    void worker_thread() {
        while (true) {
            PriorityTask task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this]() {
                    return stop_ || !tasks_.empty();
                });
                
                if (stop_ && tasks_.empty()) {
                    return;
                }
                
                task = std::move(const_cast<PriorityTask&>(tasks_.top()));
                tasks_.pop();
            }
            
            task.func();
        }
    }
    
    /**
     * 执行任务并统计
     */
    void execute_task(std::function<void()> task) {
        auto start = enable_stats_ ? std::chrono::steady_clock::now() 
                                    : std::chrono::steady_clock::time_point{};
        
        try {
            task();
            if (enable_stats_) {
                completed_tasks_++;
            }
        } catch (...) {
            if (enable_stats_) {
                failed_tasks_++;
            }
            // 静默失败，不抛出异常
        }
        
        if (enable_stats_) {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end - start).count();
            total_execution_time_ += duration;
        }
    }
    
    std::vector<std::thread> workers_;
    std::priority_queue<PriorityTask> tasks_;
    
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
    
    // 统计
    bool enable_stats_;
    std::atomic<size_t> total_tasks_;
    std::atomic<size_t> completed_tasks_;
    std::atomic<size_t> failed_tasks_;
    std::atomic<long long> total_execution_time_;
    std::chrono::steady_clock::time_point creation_time_;
};

} // namespace thread_pool
