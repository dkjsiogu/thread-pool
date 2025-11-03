#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <vector>
#include <atomic>
#include <chrono>
#include <future>
namespace thread_pool
{
    class MyThreadPool
    {
    public:
        explicit MyThreadPool(size_t num_threads = std::thread::hardware_concurrency());
        ~MyThreadPool();
        template <typename F, typename... Args>
        auto submit(F &&f, Args &&...args)
            -> std::future<typename std::result_of<F(Args...)>::type>
        {
            using return_type = typename std::result_of<F(Args...)>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> result = task->get_future();
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                if (stop_)
                {
                    throw std::runtime_error("stopped");
                }
                active_tasks_++;
                tasks_.emplace([this, task]() { 
                    (*task)();
                });
            }
            condition_.notify_one();
            return result;
        }
        size_t pending_tasks() const;

        void wait_all();

    private:
        std::vector<std::thread> workers_;

        std::queue<std::function<void()>> tasks_;
        std::condition_variable condition_;
        mutable std::mutex queue_mutex_; // mutable: 允许在 const 函数中修改
        bool stop_;

        std::atomic<int> active_tasks_{0};

        std::condition_variable all_tasks_done_;
    };

} // namespace thread_pool