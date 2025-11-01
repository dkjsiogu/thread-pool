#include "ThreadPool.h"
#include <iostream>
#include <vector>
#include <numeric>

using namespace thread_pool;

/**
 * 最终版本：生产级线程池的完整示例
 */

void print_separator(const std::string& title = "") {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    if (!title.empty()) {
        std::cout << "  " << title << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
}

int main() {
    std::cout << "🎯 生产级线程池 - 完整示例" << std::endl;
    
    // ==================== 基础使用 ====================
    print_separator("1. 基础使用");
    
    ThreadPool pool(4);
    
    // 提交简单任务
    auto future1 = pool.submit([]() {
        return 42;
    });
    
    std::cout << "结果: " << future1.get() << std::endl;
    
    // ==================== 带参数的任务 ====================
    print_separator("2. 带参数的任务");
    
    auto add = [](int a, int b) {
        return a + b;
    };
    
    auto future2 = pool.submit(add, 10, 20);
    std::cout << "10 + 20 = " << future2.get() << std::endl;
    
    // ==================== 优先级任务 ====================
    print_separator("3. 优先级任务");
    
    pool.submit(Priority::LOW, []() {
        std::cout << "  🟢 低优先级" << std::endl;
    });
    
    pool.submit(Priority::CRITICAL, []() {
        std::cout << "  🔴 紧急任务！" << std::endl;
    });
    
    pool.submit(Priority::NORMAL, []() {
        std::cout << "  🟡 普通任务" << std::endl;
    });
    
    pool.submit(Priority::HIGH, []() {
        std::cout << "  🟠 高优先级" << std::endl;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // ==================== 批量任务处理 ====================
    print_separator("4. 批量任务处理");
    
    std::vector<int> numbers(100);
    std::iota(numbers.begin(), numbers.end(), 1);  // 1, 2, 3, ..., 100
    
    // 将任务分成4份
    const size_t chunk_size = 25;
    std::vector<std::future<long long>> results;
    
    for (size_t i = 0; i < numbers.size(); i += chunk_size) {
        auto future = pool.submit([&numbers, i, chunk_size]() -> long long {
            long long sum = 0;
            for (size_t j = i; j < std::min(i + chunk_size, numbers.size()); ++j) {
                sum += numbers[j];
            }
            return sum;
        });
        results.push_back(std::move(future));
    }
    
    long long total_sum = 0;
    for (auto& f : results) {
        total_sum += f.get();
    }
    
    std::cout << "并行计算 1+2+...+100 = " << total_sum << std::endl;
    
    // ==================== 异常处理 ====================
    print_separator("5. 异常处理");
    
    auto future_error = pool.submit([]() -> int {
        throw std::runtime_error("测试异常");
        return 0;
    });
    
    try {
        future_error.get();
    } catch (const std::exception& e) {
        std::cout << "✓ 捕获到异常: " << e.what() << std::endl;
    }
    
    // ==================== 实际应用：图像处理模拟 ====================
    print_separator("6. 实际应用：图像处理模拟");
    
    struct Image {
        int id;
        std::vector<int> pixels;
        
        Image(int i, size_t size) : id(i), pixels(size, 0) {}
    };
    
    auto process_image = [](Image img) -> Image {
        // 模拟图像处理（例如滤镜）
        for (auto& pixel : img.pixels) {
            pixel = pixel * 2 + 1;  // 简单处理
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return img;
    };
    
    std::vector<Image> images;
    for (int i = 0; i < 10; ++i) {
        images.emplace_back(i, 1000);
    }
    
    std::cout << "处理 " << images.size() << " 张图片..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<Image>> processed_images;
    for (auto& img : images) {
        processed_images.push_back(
            pool.submit(Priority::HIGH, process_image, img)
        );
    }
    
    // 等待所有图片处理完成
    for (auto& future : processed_images) {
        future.get();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    std::cout << "✓ 所有图片处理完成，耗时: " << duration << " ms" << std::endl;
    
    // ==================== 实际应用：网络请求模拟 ====================
    print_separator("7. 实际应用：并发网络请求模拟");
    
    auto fetch_url = [](const std::string& url) -> std::string {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return "Response from " + url;
    };
    
    std::vector<std::string> urls = {
        "http://example.com/api/1",
        "http://example.com/api/2",
        "http://example.com/api/3",
        "http://example.com/api/4",
        "http://example.com/api/5"
    };
    
    std::cout << "发起 " << urls.size() << " 个并发请求..." << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<std::string>> responses;
    for (const auto& url : urls) {
        responses.push_back(pool.submit(fetch_url, url));
    }
    
    for (auto& response : responses) {
        std::cout << "  ✓ " << response.get() << std::endl;
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    std::cout << "✓ 所有请求完成，耗时: " << duration << " ms" << std::endl;
    std::cout << "  (串行执行约需: " << urls.size() * 200 << " ms)" << std::endl;
    
    // ==================== 性能统计 ====================
    print_separator("8. 性能统计");
    
    std::cout << "线程池信息:" << std::endl;
    std::cout << "  线程数: " << pool.thread_count() << std::endl;
    std::cout << "  总任务数: " << pool.total_tasks() << std::endl;
    std::cout << "  已完成: " << pool.completed_tasks() << std::endl;
    std::cout << "  失败: " << pool.failed_tasks() << std::endl;
    std::cout << "  待处理: " << pool.pending_tasks() << std::endl;
    std::cout << "  运行时间: " << pool.uptime() << " 秒" << std::endl;
    std::cout << "  平均执行时间: " << pool.average_execution_time() << " 微秒" << std::endl;
    
    // ==================== 总结 ====================
    print_separator("总结");
    
    std::cout << "\n✅ 你已经掌握了线程池的所有核心概念！\n" << std::endl;
    std::cout << "核心知识点回顾：" << std::endl;
    std::cout << "  1. 线程池基本原理（工作队列 + 工作线程）" << std::endl;
    std::cout << "  2. 任务提交与执行" << std::endl;
    std::cout << "  3. std::future 获取返回值" << std::endl;
    std::cout << "  4. 任务优先级" << std::endl;
    std::cout << "  5. 异常处理" << std::endl;
    std::cout << "  6. 性能统计" << std::endl;
    std::cout << "  7. 优雅关闭" << std::endl;
    
    std::cout << "\n🎓 下一步学习建议：" << std::endl;
    std::cout << "  - 研究 C++17 的并行算法" << std::endl;
    std::cout << "  - 学习无锁编程和原子操作" << std::endl;
    std::cout << "  - 了解工作窃取（work stealing）" << std::endl;
    std::cout << "  - 研究 Boost.Asio 等高级库" << std::endl;
    
    std::cout << "\n💡 实际应用场景：" << std::endl;
    std::cout << "  - Web 服务器请求处理" << std::endl;
    std::cout << "  - 图像/视频批量处理" << std::endl;
    std::cout << "  - 科学计算并行化" << std::endl;
    std::cout << "  - 日志异步写入" << std::endl;
    std::cout << "  - 网络 I/O 并发" << std::endl;
    
    print_separator();
    
    std::cout << "\n准备关闭线程池..." << std::endl;
    pool.shutdown();
    std::cout << "✓ 线程池已关闭" << std::endl;
    
    return 0;
}
