#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <vector>
#include <atomic>
#include <chrono>
namespace thread_pool {
class MyThreadPool {
    public:

    explicit MyThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~MyThreadPool();
    void submit(std::function<void()> task);
    size_t pending_tasks() const;
    private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::condition_variable condition_;
    mutable std::mutex queue_mutex_;  // mutable: 允许在 const 函数中修改
    bool stop_;

    };
} // namespace thread_pool`