# 📝 第一阶段 - 自我测试

## ✅ 知识点检测

### 理论理解（必须全部理解）

回答以下问题，确保你真正理解了概念：

#### 1. 线程池的基本概念
- [ ] **问题**: 什么是线程池？它解决了什么问题？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 线程池是预先创建一组工作线程，通过任务队列复用这些线程来执行任务。
  
  **解决的问题**:
  - 避免频繁创建/销毁线程的开销
  - 控制并发数量，防止资源耗尽
  - 提高响应速度（线程已就绪）
  </details>

- [ ] **问题**: 生产者-消费者模式是什么？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 
  - **生产者**: submit() 函数，将任务放入队列
  - **消费者**: 工作线程，从队列取任务并执行
  - **缓冲区**: 任务队列 (std::queue)
  </details>

#### 2. 同步机制
- [ ] **问题**: `std::mutex` 的作用是什么？为什么需要它？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 互斥锁用于保护共享资源（任务队列），确保同一时间只有一个线程访问，防止数据竞争。
  </details>

- [ ] **问题**: `std::condition_variable` 解决了什么问题？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 
  - 让线程高效地等待某个条件成立
  - 避免忙等待（busy waiting），节省 CPU
  - 用于线程间通信（生产者通知消费者）
  </details>

- [ ] **问题**: `condition_.wait()` 做了什么？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 
  1. 原子地释放锁
  2. 阻塞线程，等待通知
  3. 被唤醒后，重新获取锁
  4. 检查条件是否满足
  </details>

#### 3. 代码理解
- [ ] **问题**: 为什么在执行任务时不持有锁？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 如果执行任务时持有锁，会阻塞其他线程提交新任务或取任务，降低并发性能。
  
  正确做法：
  ```cpp
  {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      task = tasks_.front();  // 持有锁
      tasks_.pop();
  }  // 锁自动释放
  
  task();  // 不持有锁，执行任务
  ```
  </details>

- [ ] **问题**: 析构函数中为什么要设置 `stop_ = true` 并调用 `notify_all()`？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 
  - `stop_ = true`: 告诉线程该退出了
  - `notify_all()`: 唤醒所有等待的线程，让它们检查 stop_ 标志
  - `join()`: 等待所有线程真正退出
  </details>

---

## 💻 编码测试（必须能独立完成）

### 测试 1: 基础使用
不看代码，尝试写出创建线程池并提交任务的代码：

```cpp
// 创建一个包含 8 个线程的线程池


// 提交一个打印 "Hello World" 的任务

```

<details>
<summary>查看答案</summary>

```cpp
ThreadPool pool(8);

pool.submit([]() {
    std::cout << "Hello World" << std::endl;
});
```
</details>

---

### 测试 2: 理解任务队列
**问题**: 如果线程池有 4 个线程，你提交了 10 个任务，会发生什么？

- [ ] A. 只执行前 4 个任务
- [ ] B. 报错
- [ ] C. 前 4 个任务立即执行，剩余 6 个在队列等待
- [ ] D. 创建 10 个线程

<details>
<summary>查看答案</summary>

**答案**: C

**解释**: 
- 4 个线程会立即开始执行前 4 个任务
- 剩余 6 个任务在队列中等待
- 当某个线程完成任务后，会从队列取下一个任务
</details>

---

### 测试 3: 线程安全
看下面的代码，找出问题：

```cpp
ThreadPool pool(4);
int counter = 0;  // 共享变量

for (int i = 0; i < 1000; ++i) {
    pool.submit([&counter]() {
        counter++;  // ❌ 问题在哪里？
    });
}

std::this_thread::sleep_for(std::chrono::seconds(1));
std::cout << counter << std::endl;
```

**问题**: 
- [ ] 这段代码有什么问题？
- [ ] 如何修复？

<details>
<summary>查看答案</summary>

**问题**: 数据竞争！多个线程同时访问 `counter`，没有同步保护。

**修复方法 1** - 使用互斥锁：
```cpp
std::mutex counter_mutex;

for (int i = 0; i < 1000; ++i) {
    pool.submit([&counter, &counter_mutex]() {
        std::lock_guard<std::mutex> lock(counter_mutex);
        counter++;
    });
}
```

**修复方法 2** - 使用原子变量：
```cpp
std::atomic<int> counter(0);

for (int i = 0; i < 1000; ++i) {
    pool.submit([&counter]() {
        counter++;  // 原子操作，线程安全
    });
}
```
</details>

---

## 🔧 实践挑战（推荐完成）

### 挑战 1: 修改线程数量
**任务**: 修改代码，创建一个只有 1 个线程的线程池，提交 5 个任务，观察它们是否串行执行。

**期望结果**: 任务应该一个接一个执行，不会并发。

---

### 挑战 2: 自定义任务
**任务**: 写一个函数，计算斐波那契数列的第 n 项，并用线程池并行计算 fib(30) 到 fib(35)。

<details>
<summary>提示</summary>

```cpp
int fibonacci(int n) {
    if (n <= 1) return n;
    if (n == 2) return 1;
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

// 用线程池执行
ThreadPool pool(4);
for (int i = 30; i <= 35; ++i) {
    pool.submit([i]() {
        int result = fibonacci(i);
        std::cout << "fib(" << i << ") = " << result << std::endl;
    });
}
```
</details>

---

### 挑战 3: 等待所有任务完成
**问题**: 当前的线程池没有提供"等待所有任务完成"的功能，你能想办法实现吗？

<details>
<summary>提示</summary>

方法 1: 简单等待（不精确）
```cpp
std::this_thread::sleep_for(std::chrono::seconds(5));
```

方法 2: 轮询队列（更好）
```cpp
while (pool.pending_tasks() > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

方法 3: 使用计数器（最好，但需要修改线程池）
- 添加 `std::atomic<int> active_tasks_`
- submit 时 +1，任务完成时 -1
- 提供 `wait_all()` 方法
</details>

---

## 🎯 通关标准

### ✅ 你已经掌握第一阶段，如果：

**理论部分** (必须全对):
- ✅ 能解释线程池的工作原理
- ✅ 理解生产者-消费者模式
- ✅ 掌握 mutex 和 condition_variable 的作用
- ✅ 理解为什么需要同步机制

**实践部分** (必须能做到):
- ✅ 能不看文档创建线程池
- ✅ 能提交简单的任务
- ✅ 理解线程安全问题
- ✅ 知道如何保护共享数据

**代码阅读** (必须理解):
- ✅ 能看懂 `ThreadPool.h` 的每一行代码
- ✅ 理解工作线程的生命周期
- ✅ 理解任务队列的工作流程

---

## 📊 自我评分

请诚实地给自己打分：

| 项目 | 分数 (0-10) | 说明 |
|------|-------------|------|
| 理论理解 | ___ / 10 | 能解释核心概念 |
| 代码理解 | ___ / 10 | 能看懂实现细节 |
| 实践能力 | ___ / 10 | 能独立编写代码 |
| 问题解决 | ___ / 10 | 能解决线程安全问题 |

**总分**: ___ / 40

### 评级标准：
- **35-40 分**: 🏆 优秀！可以进入第二阶段
- **28-34 分**: 👍 良好，建议再复习一遍代码
- **20-27 分**: 📖 及格，需要多练习
- **< 20 分**: 🔄 建议重新学习本阶段

---

## 🚀 进入下一阶段的条件

只有满足以下**所有**条件，才建议进入第二阶段：

- [ ] 理论测试全部正确
- [ ] 能独立完成编码测试
- [ ] 至少完成 2 个实践挑战
- [ ] 自我评分 ≥ 30 分
- [ ] 能向别人解释线程池的工作原理

---

## 💡 学习建议

### 如果你感觉还没掌握：
1. 重新运行 `./step1_basic_threadpool`，仔细观察输出
2. 逐行阅读 `ThreadPool.h` 的注释
3. 尝试修改代码（改变线程数、任务数）
4. 在纸上画出线程池的工作流程图
5. 向我提问你不理解的地方

### 如果你已经掌握：
🎉 恭喜！你可以继续学习第二阶段了！

运行下一个示例：
```bash
cd /home/dkjsiogu/文档/Project/thread\ pool/build
./step2_with_future
```

---

**记住**: 编程不是比速度，理解比记忆更重要！宁愿慢慢学扎实，也不要囫囵吞枣。
