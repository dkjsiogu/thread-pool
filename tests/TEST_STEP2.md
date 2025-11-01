# 📝 第二阶段 - 自我测试

## ✅ 知识点检测

### 理论理解（必须全部理解）

#### 1. Future 和 Promise
- [ ] **问题**: `std::future` 是什么？它解决了什么问题？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: `std::future` 代表一个异步操作的"未来"结果。
  
  **解决的问题**:
  - 第一阶段无法获取任务返回值
  - 不知道任务何时完成
  - 无法等待特定任务
  
  **使用方法**:
  ```cpp
  auto future = pool.submit([]() { return 42; });
  int result = future.get();  // 获取结果
  ```
  </details>

- [ ] **问题**: `future.get()` 和 `future.wait()` 有什么区别？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**:
  - `get()`: 等待任务完成**并获取返回值**，只能调用一次
  - `wait()`: 等待任务完成**但不获取值**，可以多次调用
  </details>

- [ ] **问题**: `std::packaged_task` 的作用是什么？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 
  - 包装一个可调用对象（函数、lambda）
  - 自动创建关联的 future
  - 执行时会自动设置 future 的值
  
  ```cpp
  std::packaged_task<int()> task([]() { return 42; });
  std::future<int> fut = task.get_future();
  task();  // 执行任务
  int result = fut.get();  // 获取结果
  ```
  </details>

#### 2. 模板编程
- [ ] **问题**: `std::result_of` 的作用是什么？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 在编译期推导函数的返回类型。
  
  ```cpp
  // F = lambda: [](int, int) { return int; }
  // Args = int, int
  // result_of<F(Args...)>::type = int
  using return_type = typename std::result_of<F(Args...)>::type;
  ```
  </details>

- [ ] **问题**: `std::forward` 的作用是什么？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 完美转发，保持参数的值类别（左值/右值）。
  
  **好处**:
  - 避免不必要的拷贝
  - 支持移动语义
  - 提高性能
  </details>

#### 3. 异常处理
- [ ] **问题**: 任务中抛出的异常会怎样？
  <details>
  <summary>点击查看答案</summary>
  
  **答案**: 
  1. 异常被 `packaged_task` 捕获
  2. 存储在关联的 future 中
  3. 调用 `future.get()` 时重新抛出
  
  ```cpp
  auto fut = pool.submit([]() {
      throw std::runtime_error("error");
      return 0;
  });
  
  try {
      fut.get();  // 异常在这里抛出
  } catch (const std::exception& e) {
      // 处理异常
  }
  ```
  </details>

---

## 💻 编码测试

### 测试 1: 基础使用
不看代码，写出提交任务并获取返回值的代码：

```cpp
// 1. 创建线程池


// 2. 提交一个返回 100 的任务


// 3. 获取并打印结果

```

<details>
<summary>查看答案</summary>

```cpp
ThreadPoolWithFuture pool(4);

auto future = pool.submit([]() {
    return 100;
});

std::cout << future.get() << std::endl;
```
</details>

---

### 测试 2: 带参数的任务
写代码：提交一个计算两数之和的任务

```cpp
// 定义函数
int add(int a, int b) {
    return a + b;
}

// 提交任务并获取结果

```

<details>
<summary>查看答案</summary>

```cpp
auto future = pool.submit(add, 10, 20);
int result = future.get();  // 30
```
</details>

---

### 测试 3: 批量任务
写代码：提交 10 个任务，每个计算 i*i，收集所有结果

```cpp
// 你的代码：



```

<details>
<summary>查看答案</summary>

```cpp
std::vector<std::future<int>> futures;

for (int i = 1; i <= 10; ++i) {
    futures.push_back(pool.submit([i]() {
        return i * i;
    }));
}

for (auto& fut : futures) {
    std::cout << fut.get() << " ";
}
```
</details>

---

### 测试 4: 异常处理
这段代码会输出什么？

```cpp
auto fut = pool.submit([]() -> int {
    throw std::runtime_error("Oops!");
    return 42;
});

std::cout << "A" << std::endl;
int result = fut.get();
std::cout << "B" << std::endl;
```

- [ ] A. 输出 "A"，然后崩溃
- [ ] B. 输出 "A"，然后抛出异常
- [ ] C. 什么都不输出
- [ ] D. 输出 "A" 和 "B"

<details>
<summary>查看答案</summary>

**答案**: B

**解释**:
1. 任务提交成功
2. 输出 "A"
3. `fut.get()` 会阻塞等待
4. 任务执行时抛出异常
5. `fut.get()` 重新抛出异常
6. 如果没有 try-catch，程序会终止

**正确写法**:
```cpp
try {
    int result = fut.get();
    std::cout << "B" << std::endl;
} catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
}
```
</details>

---

### 测试 5: wait_for 的使用
填空：

```cpp
auto fut = pool.submit([]() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 42;
});

// 等待 500ms
auto status = fut._____(std::chrono::milliseconds(500));

if (status == std::future_status::_____) {
    std::cout << "任务还在执行" << std::endl;
} else if (status == std::future_status::_____) {
    std::cout << "任务完成" << std::endl;
}
```

<details>
<summary>查看答案</summary>

```cpp
auto status = fut.wait_for(std::chrono::milliseconds(500));

if (status == std::future_status::timeout) {
    std::cout << "任务还在执行" << std::endl;
} else if (status == std::future_status::ready) {
    std::cout << "任务完成" << std::endl;
}
```
</details>

---

## 🔧 实践挑战

### 挑战 1: 并行文件处理
**任务**: 模拟处理 5 个文件，每个文件返回字数统计

```cpp
int count_words(const std::string& filename) {
    // 模拟处理
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return filename.length() * 100;  // 假装统计
}

// 使用线程池并行处理多个文件
// 你的代码：
```

<details>
<summary>查看答案</summary>

```cpp
ThreadPoolWithFuture pool(4);
std::vector<std::string> files = {
    "file1.txt", "file2.txt", "file3.txt", 
    "file4.txt", "file5.txt"
};

std::vector<std::future<int>> results;
for (const auto& file : files) {
    results.push_back(pool.submit(count_words, file));
}

int total = 0;
for (auto& fut : results) {
    total += fut.get();
}

std::cout << "总字数: " << total << std::endl;
```
</details>

---

### 挑战 2: 实现 map 函数
**任务**: 实现一个 `parallel_map` 函数，对 vector 中的每个元素应用函数

```cpp
template<typename T, typename F>
std::vector</* 返回类型 */> parallel_map(
    ThreadPoolWithFuture& pool,
    const std::vector<T>& data,
    F func
) {
    // 你的实现
}

// 测试
std::vector<int> numbers = {1, 2, 3, 4, 5};
auto squares = parallel_map(pool, numbers, [](int x) {
    return x * x;
});
// squares = {1, 4, 9, 16, 25}
```

<details>
<summary>查看答案</summary>

```cpp
template<typename T, typename F>
auto parallel_map(
    ThreadPoolWithFuture& pool,
    const std::vector<T>& data,
    F func
) -> std::vector<typename std::result_of<F(T)>::type> {
    
    using ReturnType = typename std::result_of<F(T)>::type;
    
    std::vector<std::future<ReturnType>> futures;
    for (const auto& item : data) {
        futures.push_back(pool.submit(func, item));
    }
    
    std::vector<ReturnType> results;
    for (auto& fut : futures) {
        results.push_back(fut.get());
    }
    
    return results;
}
```
</details>

---

## 🎯 通关标准

### ✅ 你已经掌握第二阶段，如果：

**理论部分**:
- ✅ 理解 future/promise 机制
- ✅ 掌握 packaged_task 的用法
- ✅ 理解异步异常处理
- ✅ 了解模板编程基础

**实践部分**:
- ✅ 能提交带返回值的任务
- ✅ 能处理批量任务结果
- ✅ 能正确处理异常
- ✅ 能使用 wait_for 检查状态

**代码理解**:
- ✅ 理解 submit 函数的模板实现
- ✅ 理解 future 和 packaged_task 的关系
- ✅ 理解完美转发的作用

---

## 📊 自我评分

| 项目 | 分数 (0-10) |
|------|-------------|
| Future/Promise 理解 | ___ / 10 |
| 模板编程理解 | ___ / 10 |
| 异常处理能力 | ___ / 10 |
| 实践编码能力 | ___ / 10 |

**总分**: ___ / 40

### 评级：
- **35-40**: 🏆 优秀！可以进入第三阶段
- **28-34**: 👍 良好，建议多练习
- **< 28**: 📖 需要重新学习

---

## 🚀 准备好了吗？

满足以下条件可进入第三阶段：
- [ ] 理论测试全部正确
- [ ] 完成所有编码测试
- [ ] 至少完成 1 个实践挑战
- [ ] 自我评分 ≥ 28 分

**下一步**: 运行 `./step3_advanced` 学习优先级、统计等高级特性！
