#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <string>

/**
 * 第一阶段示例：基础线程池的使用
 */

// 示例任务：模拟一些耗时操作
void example_task(int task_id, int sleep_ms) {
    std::cout << "  📝 任务 #" << task_id << " 开始执行 (线程 ID: " 
              << std::this_thread::get_id() << ")" << std::endl;
    
    // 模拟耗时操作
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    
    std::cout << "  ✓ 任务 #" << task_id << " 完成" << std::endl;
}

int main() {
    std::cout << "=== 第一阶段：基础线程池 ===" << std::endl;
    std::cout << "\n📚 学习目标：" << std::endl;
    std::cout << "  1. 理解线程池的基本工作原理" << std::endl;
    std::cout << "  2. 掌握任务队列的使用" << std::endl;
    std::cout << "  3. 了解生产者-消费者模式" << std::endl;
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    
    // ==================== 示例 1：创建线程池 ====================
    std::cout << "【示例 1】创建包含 4 个线程的线程池\n" << std::endl;
    ThreadPool pool(4);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // ==================== 示例 2：提交简单任务 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 2】提交 8 个简单任务\n" << std::endl;
    
    for (int i = 1; i <= 8; ++i) {
        pool.submit([i]() {
            example_task(i, 500);
        });
    }
    
    std::cout << "\n💡 注意观察：" << std::endl;
    std::cout << "  - 多个任务可能并发执行" << std::endl;
    std::cout << "  - 同一个线程可能执行多个任务" << std::endl;
    
    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // ==================== 示例 3：使用 Lambda 表达式 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 3】使用 Lambda 表达式提交任务\n" << std::endl;
    
    pool.submit([]() {
        std::cout << "  🎯 这是一个 lambda 任务！" << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::cout << "    计数: " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // ==================== 示例 4：任务队列 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 4】观察任务队列\n" << std::endl;
    
    // 快速提交大量任务
    for (int i = 1; i <= 20; ++i) {
        pool.submit([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << "  快速任务 #" << i << " 完成" << std::endl;
        });
    }
    
    std::cout << "当前待处理任务数: " << pool.pending_tasks() << std::endl;
    std::cout << "\n💡 说明：" << std::endl;
    std::cout << "  - 任务数超过线程数时，多余的任务会在队列中等待" << std::endl;
    std::cout << "  - 线程完成当前任务后，会自动从队列取下一个任务" << std::endl;
    
    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // ==================== 示例 5：共享数据 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 5】多个任务共享数据（注意线程安全！）\n" << std::endl;
    
    // ⚠️ 注意：这个示例有线程安全问题！（仅作演示）
    int shared_counter = 0;
    std::mutex counter_mutex;  // 保护共享计数器
    
    for (int i = 0; i < 10; ++i) {
        pool.submit([&shared_counter, &counter_mutex]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // 使用互斥锁保护共享数据
            std::lock_guard<std::mutex> lock(counter_mutex);
            shared_counter++;
            std::cout << "  计数器: " << shared_counter << std::endl;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "\n最终计数器值: " << shared_counter << std::endl;
    
    // ==================== 总结 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "📖 第一阶段总结：\n" << std::endl;
    std::cout << "✓ 你已经学会了：" << std::endl;
    std::cout << "  1. 创建和使用基础线程池" << std::endl;
    std::cout << "  2. 使用 submit() 提交任务" << std::endl;
    std::cout << "  3. 理解任务队列的工作原理" << std::endl;
    std::cout << "  4. 注意多线程中的数据共享问题" << std::endl;
    std::cout << "\n⚠️ 当前版本的局限：" << std::endl;
    std::cout << "  1. 不支持获取任务的返回值" << std::endl;
    std::cout << "  2. 无法知道任务何时完成" << std::endl;
    std::cout << "  3. 异常处理不够完善" << std::endl;
    std::cout << "\n🚀 下一阶段预告：" << std::endl;
    std::cout << "  我们将使用 std::future 来支持任务返回值！" << std::endl;
    std::cout << "\n" << std::string(50, '=') << std::endl;
    
    std::cout << "\n准备关闭线程池..." << std::endl;
    
    // ThreadPool 的析构函数会自动被调用，关闭所有线程
    return 0;
}

/**
 * 💡 练习建议：
 * 
 * 1. 修改线程池大小，观察任务执行的变化
 * 2. 尝试提交不同类型的任务
 * 3. 思考：如果任务会抛出异常会怎样？
 * 4. 思考：如何获取任务的返回值？
 * 
 * 这些问题我们会在后续阶段解决！
 */
