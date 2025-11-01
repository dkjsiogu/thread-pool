#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <string>
#include <random>

/**
 * 第三阶段示例：高级功能的线程池
 */

int main() {
    std::cout << "=== 第三阶段：高级功能的线程池 ===" << std::endl;
    std::cout << "\n📚 学习目标：" << std::endl;
    std::cout << "  1. 使用任务优先级" << std::endl;
    std::cout << "  2. 查看性能统计信息" << std::endl;
    std::cout << "  3. 理解不同的关闭机制" << std::endl;
    std::cout << "  4. 处理任务失败" << std::endl;
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    
    AdvancedThreadPool pool(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // ==================== 示例 1：任务优先级 ====================
    std::cout << "【示例 1】任务优先级演示\n" << std::endl;
    
    // 提交不同优先级的任务
    pool.submit(Priority::LOW, []() {
        std::cout << "  🟢 低优先级任务执行" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    pool.submit(Priority::CRITICAL, []() {
        std::cout << "  🔴 紧急任务执行！" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    pool.submit(Priority::NORMAL, []() {
        std::cout << "  🟡 普通任务执行" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    pool.submit(Priority::HIGH, []() {
        std::cout << "  🟠 高优先级任务执行" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    std::cout << "\n💡 观察：紧急任务会优先执行！\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // ==================== 示例 2：批量任务演示 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 2】批量提交不同优先级的任务\n" << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3);
    
    for (int i = 1; i <= 20; ++i) {
        Priority p = static_cast<Priority>(dis(gen));
        pool.submit(p, [i, p]() {
            std::string priority_str;
            switch (p) {
                case Priority::LOW: priority_str = "🟢 LOW"; break;
                case Priority::NORMAL: priority_str = "🟡 NORMAL"; break;
                case Priority::HIGH: priority_str = "🟠 HIGH"; break;
                case Priority::CRITICAL: priority_str = "🔴 CRITICAL"; break;
            }
            
            std::cout << "  任务 #" << i << " [" << priority_str << "]" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });
    }
    
    std::cout << "\n当前待处理任务: " << pool.pending_tasks() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // ==================== 示例 3：异常处理 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 3】任务异常处理\n" << std::endl;
    
    // 正常任务
    auto future1 = pool.submit([]() {
        std::cout << "  ✓ 正常任务" << std::endl;
        return 100;
    });
    
    // 会抛出异常的任务
    auto future2 = pool.submit([]() -> int {
        std::cout << "  💥 即将抛出异常..." << std::endl;
        throw std::runtime_error("故意抛出的异常");
        return 200;
    });
    
    // 另一个正常任务
    auto future3 = pool.submit([]() {
        std::cout << "  ✓ 另一个正常任务" << std::endl;
        return 300;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 获取结果
    try {
        std::cout << "\n获取结果1: " << future1.get() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "捕获异常: " << e.what() << std::endl;
    }
    
    try {
        std::cout << "获取结果2: " << future2.get() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "捕获异常: " << e.what() << std::endl;
    }
    
    try {
        std::cout << "获取结果3: " << future3.get() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "捕获异常: " << e.what() << std::endl;
    }
    
    std::cout << "\n💡 关键点：" << std::endl;
    std::cout << "  - 任务中的异常不会导致线程池崩溃" << std::endl;
    std::cout << "  - 异常会被捕获并记录" << std::endl;
    std::cout << "  - 其他任务不受影响" << std::endl;
    
    // ==================== 示例 4：性能统计 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 4】性能统计\n" << std::endl;
    
    // 提交一批计算密集型任务
    std::cout << "提交 50 个计算任务...\n" << std::endl;
    
    for (int i = 0; i < 50; ++i) {
        pool.submit([i]() {
            // 模拟计算
            long long sum = 0;
            for (int j = 0; j < 1000000; ++j) {
                sum += j;
            }
        });
    }
    
    std::cout << "等待任务完成...\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 显示统计信息
    pool.print_statistics();
    
    // ==================== 示例 5：返回值 + 优先级 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 5】组合使用：返回值 + 优先级\n" << std::endl;
    
    auto urgent_result = pool.submit(Priority::CRITICAL, [](int x, int y) {
        std::cout << "  🔴 紧急计算: " << x << " * " << y << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return x * y;
    }, 10, 20);
    
    auto normal_result = pool.submit(Priority::NORMAL, [](int x, int y) {
        std::cout << "  🟡 普通计算: " << x << " + " << y << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return x + y;
    }, 10, 20);
    
    std::cout << "\n✓ 紧急任务结果: " << urgent_result.get() << std::endl;
    std::cout << "✓ 普通任务结果: " << normal_result.get() << std::endl;
    
    // ==================== 示例 6：关闭机制对比 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "【示例 6】演示不同的关闭机制\n" << std::endl;
    
    std::cout << "\n创建临时线程池（测试优雅关闭）..." << std::endl;
    {
        AdvancedThreadPool temp_pool(2);
        
        // 提交一些任务
        for (int i = 1; i <= 5; ++i) {
            temp_pool.submit([i]() {
                std::cout << "  临时任务 #" << i << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            });
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "\n离开作用域，线程池将优雅关闭（等待所有任务完成）..." << std::endl;
        // temp_pool 的析构函数会被调用
    }
    
    std::cout << "\n创建另一个临时线程池（测试强制关闭）..." << std::endl;
    {
        AdvancedThreadPool temp_pool2(2);
        
        // 提交一些任务
        for (int i = 1; i <= 10; ++i) {
            temp_pool2.submit([i]() {
                std::cout << "  临时任务 #" << i << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            });
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "\n调用 shutdown_now()..." << std::endl;
        temp_pool2.shutdown_now();  // 强制关闭，丢弃未执行的任务
    }
    
    // ==================== 总结 ====================
    std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;
    std::cout << "📖 第三阶段总结：\n" << std::endl;
    std::cout << "✓ 你已经学会了：" << std::endl;
    std::cout << "  1. 使用任务优先级控制执行顺序" << std::endl;
    std::cout << "  2. 查看和分析性能统计信息" << std::endl;
    std::cout << "  3. 优雅关闭 vs 强制关闭" << std::endl;
    std::cout << "  4. 异常安全的任务执行" << std::endl;
    std::cout << "  5. 组合使用返回值和优先级" << std::endl;
    std::cout << "\n💎 这是一个功能完整的生产级线程池！" << std::endl;
    std::cout << "\n🚀 下一步：" << std::endl;
    std::cout << "  查看 final/ 目录，了解如何将线程池封装成易用的库！" << std::endl;
    std::cout << "\n" << std::string(50, '=') << std::endl;
    
    std::cout << "\n准备关闭主线程池..." << std::endl;
    pool.shutdown();
    
    return 0;
}

/**
 * 💡 高级练习：
 * 
 * 1. 尝试实现动态调整线程数量的功能
 * 2. 添加任务超时机制
 * 3. 实现任务依赖（一个任务完成后才能执行另一个）
 * 4. 添加任务取消功能
 * 5. 实现工作窃取（work stealing）算法
 * 
 * 这些都是生产级线程池可能需要的高级特性！
 */
