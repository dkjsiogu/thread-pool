#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <string>
#include <vector>

/**
 * 第二阶段示例：支持返回值的线程池
 */

// 示例：有返回值的函数
int calculate_sum(int a, int b) {
    std::cout << "  🧮 计算 " << a << " + " << b << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return a + b;
}

// 示例：返回字符串的函数
std::string process_data(int id, const std::string& data) {
    std::cout << "  📊 处理数据 #" << id << ": " << data << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    return "处理完成: " + data + " (ID: " + std::to_string(id) + ")";
}

// 示例：可能抛出异常的函数
int risky_operation(int value) {
    std::cout << "  ⚠️  执行危险操作，输入值: " << value << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    if (value < 0) {
        throw std::runtime_error("值不能为负数！");
    }
    
    return value * 2;
}

int main() {
    std::cout << "=== 第二阶段：支持返回值的线程池 ===" << std::endl;
    std::cout << "\n📚 学习目标：" << std::endl;
    std::cout << "  1. 使用 std::future 获取任务返回值" << std::endl;
    std::cout << "  2. 理解 std::packaged_task 的作用" << std::endl;
    std::cout << "  3. 掌握异步编程的基本概念" << std::endl;
    std::cout << "  4. 学习异常处理" << std::endl;
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    
    ThreadPoolWithFuture pool(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // ==================== 示例 1：获取返回值 ====================
    std::cout << "【示例 1】获取任务的返回值\n" << std::endl;
    
    // 提交任务并获取 future
    auto future1 = pool.submit(calculate_sum, 10, 20);
    auto future2 = pool.submit(calculate_sum, 5, 15);
    
    std::cout << "任务已提交，等待结果...\n" << std::endl;
    
    // 获取结果（会阻塞直到任务完成）
    int result1 = future1.get();
    int result2 = future2.get();
    
    std::cout << "✓ 结果1: " << result1 << std::endl;
    std::cout << "✓ 结果2: " << result2 << std::endl;
    
    std::cout << "\n💡 关键点：" << std::endl;
    std::cout << "  - future.get() 会阻塞，直到任务完成" << std::endl;
    std::cout << "  - 每个 future 只能 get() 一次" << std::endl;
    
    // ==================== 示例 2：Lambda 表达式 + 返回值 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 2】使用 Lambda 表达式\n" << std::endl;
    
    auto future3 = pool.submit([]() {
        std::cout << "  🎯 Lambda 任务开始" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 42;
    });
    
    auto future4 = pool.submit([](int x, int y) {
        return x * y;
    }, 6, 7);
    
    std::cout << "✓ Lambda 结果1: " << future3.get() << std::endl;
    std::cout << "✓ Lambda 结果2: " << future4.get() << std::endl;
    
    // ==================== 示例 3：批量任务 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 3】批量提交任务并收集结果\n" << std::endl;
    
    std::vector<std::future<std::string>> futures;
    
    for (int i = 1; i <= 5; ++i) {
        futures.push_back(
            pool.submit(process_data, i, "数据_" + std::to_string(i))
        );
    }
    
    std::cout << "\n已提交 " << futures.size() << " 个任务，收集结果：\n" << std::endl;
    
    for (auto& future : futures) {
        std::cout << "  ✓ " << future.get() << std::endl;
    }
    
    // ==================== 示例 4：wait 和 wait_for ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 4】使用 wait() 和 wait_for()\n" << std::endl;
    
    auto future5 = pool.submit([]() {
        std::cout << "  ⏰ 长时间运行的任务..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return std::string("任务完成！");
    });
    
    std::cout << "检查任务状态..." << std::endl;
    
    // 等待 500ms
    if (future5.wait_for(std::chrono::milliseconds(500)) == std::future_status::timeout) {
        std::cout << "  ⏳ 任务还在执行中..." << std::endl;
    }
    
    // 等待任务完成（不获取结果）
    future5.wait();
    std::cout << "  ✓ 任务已完成！" << std::endl;
    
    // 现在获取结果
    std::cout << "  结果: " << future5.get() << std::endl;
    
    std::cout << "\n💡 说明：" << std::endl;
    std::cout << "  - wait() 等待完成但不获取结果" << std::endl;
    std::cout << "  - wait_for() 可以设置超时时间" << std::endl;
    std::cout << "  - get() 既等待又获取结果" << std::endl;
    
    // ==================== 示例 5：异常处理 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 5】处理任务中的异常\n" << std::endl;
    
    auto future_ok = pool.submit(risky_operation, 10);
    auto future_error = pool.submit(risky_operation, -5);
    
    // 正常任务
    try {
        int result = future_ok.get();
        std::cout << "  ✓ 正常结果: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ 捕获异常: " << e.what() << std::endl;
    }
    
    // 会抛出异常的任务
    try {
        int result = future_error.get();
        std::cout << "  ✓ 结果: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ 捕获异常: " << e.what() << std::endl;
    }
    
    std::cout << "\n💡 关键点：" << std::endl;
    std::cout << "  - 任务中的异常会被捕获" << std::endl;
    std::cout << "  - future.get() 会重新抛出异常" << std::endl;
    std::cout << "  - 必须用 try-catch 处理" << std::endl;
    
    // ==================== 示例 6：性能对比 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 6】并行计算性能对比\n" << std::endl;
    
    auto compute_heavy = [](int n) {
        long long sum = 0;
        for (int i = 0; i < n; ++i) {
            sum += i * i;
        }
        return sum;
    };
    
    const int iterations = 100000000;
    
    // 串行执行
    auto start = std::chrono::high_resolution_clock::now();
    long long sum1 = compute_heavy(iterations);
    long long sum2 = compute_heavy(iterations);
    long long sum3 = compute_heavy(iterations);
    long long sum4 = compute_heavy(iterations);
    auto end = std::chrono::high_resolution_clock::now();
    auto serial_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "串行执行时间: " << serial_time << " ms" << std::endl;
    
    // 并行执行
    start = std::chrono::high_resolution_clock::now();
    auto f1 = pool.submit(compute_heavy, iterations);
    auto f2 = pool.submit(compute_heavy, iterations);
    auto f3 = pool.submit(compute_heavy, iterations);
    auto f4 = pool.submit(compute_heavy, iterations);
    f1.get(); f2.get(); f3.get(); f4.get();
    end = std::chrono::high_resolution_clock::now();
    auto parallel_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "并行执行时间: " << parallel_time << " ms" << std::endl;
    std::cout << "加速比: " << (double)serial_time / parallel_time << "x" << std::endl;
    
    // ==================== 总结 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "📖 第二阶段总结：\n" << std::endl;
    std::cout << "✓ 你已经学会了：" << std::endl;
    std::cout << "  1. 使用 submit() 提交任务并获取 future" << std::endl;
    std::cout << "  2. 用 future.get() 获取返回值" << std::endl;
    std::cout << "  3. 用 wait() 和 wait_for() 检查任务状态" << std::endl;
    std::cout << "  4. 处理任务中的异常" << std::endl;
    std::cout << "  5. 批量处理任务结果" << std::endl;
    std::cout << "\n💪 进步：" << std::endl;
    std::cout << "  相比第一阶段，现在可以：" << std::endl;
    std::cout << "  - 获取任务的返回值" << std::endl;
    std::cout << "  - 知道任务何时完成" << std::endl;
    std::cout << "  - 处理任务中的异常" << std::endl;
    std::cout << "\n🚀 下一阶段预告：" << std::endl;
    std::cout << "  我们将添加更多高级特性，如任务优先级、性能监控等！" << std::endl;
    std::cout << "\n" << std::string(50, '=') << std::endl;
    
    std::cout << "\n准备关闭线程池..." << std::endl;
    return 0;
}

/**
 * 💡 练习建议：
 * 
 * 1. 尝试提交不同类型的任务（int, string, 自定义类等）
 * 2. 实验 wait_for() 的不同超时值
 * 3. 尝试在任务中抛出不同类型的异常
 * 4. 思考：如何实现任务的取消功能？
 * 5. 思考：如何实现任务的优先级？
 */
