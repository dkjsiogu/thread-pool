# 📝 第三阶段 - 自我测试

## ✅ 知识点检测

### 理论理解

#### 1. 优先级队列
- [ ] **问题**: 为什么要使用 `std::priority_queue` 而不是 `std::queue`？
  <details>
  <summary>答案</summary>
  
  **答案**: 
  - `std::queue`: FIFO（先进先出），无法区分重要性
  - `std::priority_queue`: 自动按优先级排序，重要任务先执行
  
  **应用场景**: 
  - 操作系统任务调度
  - 网络请求处理（VIP 优先）
  - 实时系统
  </details>

#### 2. 原子操作
- [ ] **问题**: `std::atomic` 和 `std::mutex` 的区别？
  <details>
  <summary>答案</summary>
  
  | 特性 | std::atomic | std::mutex |
  |------|-------------|------------|
  | 性能 | 更快（无锁） | 较慢（有锁） |
  | 适用 | 简单操作（计数） | 复杂操作（多步骤） |
  | 等待 | 不会阻塞 | 可能阻塞 |
  
  **使用场景**:
  ```cpp
  // ✅ 适合 atomic
  std::atomic<int> counter;
  counter++;
  
  // ❌ 不适合 atomic，需要 mutex
  std::mutex mtx;
  std::lock_guard<std::mutex> lock(mtx);
  data.push_back(value);
  data.sort();
  ```
  </details>

#### 3. 关闭机制
- [ ] **问题**: `shutdown()` 和 `shutdown_now()` 的区别？
  <details>
  <summary>答案</summary>
  
  **shutdown()** - 优雅关闭:
  - 不再接受新任务
  - 等待队列中的任务全部完成
  - 线程自然退出
  
  **shutdown_now()** - 强制关闭:
  - 清空任务队列
  - 正在执行的任务会完成
  - 未开始的任务被丢弃
  
  **使用场景**:
  - 正常关闭用 `shutdown()`
  - 紧急情况用 `shutdown_now()`
  </details>

---

## 💻 编码测试

### 测试 1: 优先级使用
写代码：提交 3 个不同优先级的任务

```cpp
// 你的代码：




```

<details>
<summary>答案</summary>

```cpp
AdvancedThreadPool pool(4);

pool.submit(Priority::LOW, []() {
    std::cout << "低优先级" << std::endl;
});

pool.submit(Priority::HIGH, []() {
    std::cout << "高优先级" << std::endl;
});

pool.submit(Priority::CRITICAL, []() {
    std::cout << "紧急任务" << std::endl;
});

// 输出顺序：CRITICAL -> HIGH -> LOW
```
</details>

---

### 测试 2: 性能统计
这段代码会输出什么？

```cpp
AdvancedThreadPool pool(4);

for (int i = 0; i < 10; ++i) {
    pool.submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
}

std::cout << "总任务: " << pool.total_tasks() << std::endl;
std::cout << "完成: " << pool.completed_tasks() << std::endl;
std::cout << "待处理: " << pool.pending_tasks() << std::endl;

std::this_thread::sleep_for(std::chrono::seconds(1));

std::cout << "完成: " << pool.completed_tasks() << std::endl;
```

<details>
<summary>答案</summary>

**第一组输出**:
- 总任务: 10
- 完成: 0 或 很少（刚提交，还没完成）
- 待处理: ~6-10（大部分在队列）

**第二组输出**:
- 完成: 10（所有任务已完成）
</details>

---

### 测试 3: 原子操作
找出这段代码的问题：

```cpp
class Counter {
    int count = 0;  // ❌ 问题
public:
    void increment() {
        count++;
    }
    int get() { return count; }
};

Counter counter;
for (int i = 0; i < 1000; ++i) {
    pool.submit([&counter]() {
        counter.increment();
    });
}
```

**如何修复？**

<details>
<summary>答案</summary>

**问题**: `count++` 不是原子操作，有数据竞争。

**修复方法 1 - 原子变量**:
```cpp
class Counter {
    std::atomic<int> count{0};  // ✅
public:
    void increment() {
        count++;  // 原子操作
    }
    int get() { return count.load(); }
};
```

**修复方法 2 - 互斥锁**:
```cpp
class Counter {
    int count = 0;
    std::mutex mtx;
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        count++;
    }
    int get() {
        std::lock_guard<std::mutex> lock(mtx);
        return count;
    }
};
```
</details>

---

### 测试 4: 异常安全
这个线程池会崩溃吗？

```cpp
AdvancedThreadPool pool(4);

pool.submit([]() {
    throw std::runtime_error("Task 1 failed");
});

pool.submit([]() {
    std::cout << "Task 2" << std::endl;
});

pool.submit([]() {
    throw std::logic_error("Task 3 failed");
});

std::this_thread::sleep_for(std::chrono::seconds(1));
pool.print_statistics();
```

<details>
<summary>答案</summary>

**不会崩溃！**

**原因**: 
- 工作线程用 try-catch 捕获所有异常
- 异常不会传播到其他线程
- `failed_tasks_` 计数器会增加
- 其他任务正常执行

**统计信息**:
- 总任务: 3
- 完成: 1 (Task 2)
- 失败: 2 (Task 1, Task 3)
</details>

---

## 🔧 实践挑战

### 挑战 1: 实现任务取消
**难度**: ⭐⭐⭐

**任务**: 修改线程池，支持取消尚未开始的任务

<details>
<summary>提示</summary>

需要添加：
1. 任务 ID 系统
2. `cancel(task_id)` 方法
3. 取消标记

```cpp
// 伪代码
struct Task {
    int id;
    std::atomic<bool> cancelled{false};
    std::function<void()> func;
};

int submit(...) {
    int task_id = next_id++;
    // 添加任务
    return task_id;
}

void cancel(int task_id) {
    // 在队列中找到并标记为取消
}
```
</details>

---

### 挑战 2: 实现 wait_all()
**难度**: ⭐⭐

**任务**: 添加一个 `wait_all()` 方法，等待所有任务完成

<details>
<summary>提示</summary>

```cpp
void wait_all() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (tasks_.empty() && active_tasks_ == 0) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
```

需要添加 `active_tasks_` 计数器。
</details>

---

### 挑战 3: 动态调整线程数
**难度**: ⭐⭐⭐⭐

**任务**: 实现根据负载自动增减线程

<details>
<summary>提示</summary>

策略：
- 如果队列积压 > 10，增加线程
- 如果线程空闲时间 > 5秒，减少线程
- 最少 2 个线程，最多 16 个

需要：
1. 监控线程（定期检查队列）
2. 动态创建/销毁线程
3. 线程状态管理
</details>

---

## 🎯 通关标准

### ✅ 掌握第三阶段需要：

**理论**:
- ✅ 理解优先级队列原理
- ✅ 掌握原子操作 vs 互斥锁
- ✅ 理解两种关闭机制
- ✅ 掌握异常安全设计

**实践**:
- ✅ 能使用优先级
- ✅ 能读取统计信息
- ✅ 能处理任务失败
- ✅ 能选择合适的关闭方式

**高级**:
- ✅ 理解线程池的完整生命周期
- ✅ 能设计线程安全的类
- ✅ 了解性能优化方向

---

## 📊 自我评分

| 项目 | 分数 (0-10) |
|------|-------------|
| 优先级队列理解 | ___ / 10 |
| 原子操作掌握 | ___ / 10 |
| 异常安全理解 | ___ / 10 |
| 综合应用能力 | ___ / 10 |

**总分**: ___ / 40

---

## 🎓 完成三阶段后

恭喜！你已经掌握了线程池的核心技术！

**你现在能够**:
- ✅ 实现功能完整的线程池
- ✅ 处理复杂的并发场景
- ✅ 优化多线程性能
- ✅ 设计线程安全的系统

**下一步**:
- 查看 `final/` 目录的完整实现
- 在实际项目中应用
- 学习更高级的并发技术

**运行最终示例**:
```bash
./final_demo
```
