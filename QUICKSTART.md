# 快速开始指南 ⚡

## 5分钟上手线程池

### 步骤 1：编译项目

```bash
cd "/home/dkjsiogu/文档/Project/thread pool"
mkdir build && cd build
cmake ..
make
```

### 步骤 2：运行示例

#### 第一阶段：基础线程池
```bash
./step1_basic_threadpool
```
**学到什么**：线程池的基本概念、任务队列、生产者-消费者模式

#### 第二阶段：支持返回值
```bash
./step2_with_future
```
**学到什么**：使用 `std::future` 获取任务结果、异常处理

#### 第三阶段：高级特性
```bash
./step3_advanced
```
**学到什么**：任务优先级、性能统计、优雅关闭

#### 最终版本：完整示例
```bash
./final_demo
```
**学到什么**：生产级线程池的实际应用

---

## 代码示例

### 最简单的使用

```cpp
#include "ThreadPool.h"

int main() {
    // 创建线程池（4个线程）
    thread_pool::ThreadPool pool(4);
    
    // 提交任务并获取结果
    auto result = pool.submit([]() {
        return 42;
    });
    
    std::cout << result.get() << std::endl;  // 输出: 42
    
    return 0;
}
```

### 带参数的任务

```cpp
auto add = [](int a, int b) { return a + b; };
auto result = pool.submit(add, 10, 20);
std::cout << result.get() << std::endl;  // 输出: 30
```

### 设置任务优先级

```cpp
pool.submit(Priority::CRITICAL, []() {
    std::cout << "紧急任务！" << std::endl;
});

pool.submit(Priority::LOW, []() {
    std::cout << "不着急的任务" << std::endl;
});
```

### 批量任务

```cpp
std::vector<std::future<int>> results;

for (int i = 0; i < 100; ++i) {
    results.push_back(pool.submit([i]() {
        return i * i;
    }));
}

for (auto& f : results) {
    std::cout << f.get() << " ";
}
```

---

## 学习路径

```
第一阶段 (基础)
    ↓
理解线程池工作原理
    ↓
第二阶段 (进阶)
    ↓
掌握异步编程 (Future/Promise)
    ↓
第三阶段 (高级)
    ↓
学习优先级、统计、关闭机制
    ↓
最终版本
    ↓
能够在实际项目中使用！
```

---

## 常见问题

### Q: 线程数量应该设置为多少？
**A**: 
- CPU密集型任务：线程数 = CPU核心数
- I/O密集型任务：线程数 = CPU核心数 × 2 或更多
- 默认使用 `std::thread::hardware_concurrency()`

### Q: 如何处理任务中的异常？
**A**: 
```cpp
auto result = pool.submit([]() {
    throw std::runtime_error("出错了！");
    return 0;
});

try {
    result.get();  // 异常会在这里重新抛出
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
}
```

### Q: 如何等待所有任务完成？
**A**:
```cpp
std::vector<std::future<void>> tasks;

// 提交任务
for (...) {
    tasks.push_back(pool.submit(...));
}

// 等待所有任务
for (auto& t : tasks) {
    t.get();  // 或 t.wait()
}
```

### Q: 程序退出时如何处理？
**A**: 
线程池的析构函数会自动调用 `shutdown()`，等待所有任务完成。
如果想强制退出，可以调用 `shutdown_now()`。

---

## 性能提示

✅ **推荐做法**:
- 任务粒度适中（不要太小）
- 避免在任务中做阻塞操作
- 使用任务优先级优化响应时间

❌ **避免**:
- 在线程池中创建新线程池（递归创建）
- 提交过多的微小任务（开销大于收益）
- 在任务中长时间持有锁

---

## 下一步

1. **运行所有示例** - 理解每个阶段的改进
2. **修改代码** - 实验不同的参数和场景
3. **实际应用** - 在自己的项目中使用
4. **深入学习** - 研究 C++ 并发编程

---

**祝学习愉快！** 🚀

有问题随时问我！
