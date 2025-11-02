# Thread Pool 教学项目 - AI 编码助手指南

## 项目概述

这是一个**渐进式 C++ 线程池教学项目**,通过 4 个阶段展示从基础到生产级的线程池实现。每个阶段都是独立可运行的完整实现,用于教学而非生产使用。

## 核心架构

### 渐进式学习结构
```
step1_basic/     → 基础概念(mutex, condition_variable, queue)
step2_future/    → 添加返回值支持(future, packaged_task)
step3_advanced/  → 高级特性(priority_queue, atomic统计)
final/           → 生产级示例(完整API + 实际应用场景)
```

每个阶段的 `ThreadPool.h` 都是**自包含的头文件**,不依赖其他阶段。

### 关键设计模式

**生产者-消费者模式**:
- `submit()` = 生产者,将任务放入队列
- 工作线程 = 消费者,从队列取任务执行
- `condition_variable` 实现等待/通知机制

**RAII 资源管理**:
- 析构函数自动调用 `shutdown()` 等待任务完成
- `unique_lock` 自动管理锁的生命周期
- 所有类禁止拷贝/移动(deleted copy/move constructors)

## 开发约定

### 代码风格

1. **注释风格**: 使用中文教学注释 + emoji 标记关键点
   ```cpp
   // ✅ 好的注释
   // 🔒 加锁：保护共享资源（任务队列和停止标志）
   // ⏰ 等待条件：有新任务到来或线程池要停止
   
   // ❌ 避免纯英文或无注释
   ```

2. **命名约定**:
   - 类名: `PascalCase` (如 `ThreadPool`)
   - 成员变量: `snake_case_` 末尾加下划线 (如 `stop_`, `queue_mutex_`)
   - 函数: `snake_case` (如 `pending_tasks()`, `shutdown_now()`)

3. **线程安全标记**: 在需要同步的地方使用明显的作用域
   ```cpp
   {
       std::unique_lock<std::mutex> lock(queue_mutex_);
       // 受保护的操作
   }
   // 锁自动释放
   ```

### 构建系统

**CMake 配置** (`CMakeLists.txt`):
- 每个阶段编译为独立可执行文件
- 要求 C++17 标准
- 自动链接 pthread
- 可执行文件命名: `step1_basic_threadpool`, `step2_with_future`, `step3_advanced`, `final_demo`

**构建命令**:
```bash
mkdir build && cd build
cmake ..
make
```

## 教学文档体系

### 文档分层
- `README.md` - 总体学习路径和编译说明
- `QUICKSTART.md` - 5分钟快速上手示例
- `LEARNING_PATH.md` - 详细学习计划和进度追踪
- `SUMMARY.md` - 教学方法和核心知识点总结
- `tests/TEST_STEP*.md` - 每个阶段的自测题(理论+实践)

### 示例代码约定
每个阶段的 `main.cpp` 包含:
1. 基础用法演示
2. 实际应用场景(如批量任务、异常处理)
3. 控制台输出显示执行过程

## 核心技术点

### Step 1: 基础同步原语
- `std::mutex` + `std::unique_lock` 保护任务队列
- `condition_.wait(lock, predicate)` 等待任务到来
- `condition_.notify_one()` 唤醒一个工作线程
- 任务类型固定为 `std::function<void()>`

### Step 2: Future/Promise 机制
- 使用 `std::result_of<F(Args...)>::type` 推导返回类型
- `std::packaged_task` 包装任务并关联 future
- `std::bind` 绑定函数参数
- 模板函数 `submit()` 支持任意可调用对象

### Step 3: 优先级和统计
- `std::priority_queue<PriorityTask>` 实现任务优先级
- `std::atomic` 无锁计数器统计性能
- `shutdown()` vs `shutdown_now()` 两种关闭策略
- 异常捕获避免工作线程崩溃

### Step 4: 生产级 API
- `namespace thread_pool` 避免命名冲突
- `enum class Priority` 类型安全的优先级
- 查询接口: `thread_count()`, `pending_tasks()`, `uptime()`
- 性能统计: `average_execution_time()`, `completed_tasks()`

## 常见修改场景

### 添加新的教学阶段
1. 在 `src/` 下创建新目录(如 `step4_xxx/`)
2. 提供 `ThreadPool.h` 和 `main.cpp`
3. 在 `CMakeLists.txt` 添加 `add_executable`
4. 更新 `README.md` 学习路径
5. 创建对应的 `tests/TEST_STEP4.md`

### 修改现有实现
- **不要破坏阶段独立性**: 每个阶段应能单独理解
- **保持教学连贯性**: 确保概念递进(简单→复杂)
- **更新相关文档**: 代码变更时同步更新注释和 README

### 调试线程池
常见问题模式:
1. **死锁**: 检查是否在持有锁时 `wait()` 或忘记 `notify()`
2. **任务丢失**: 析构时检查 `stop_` 标志设置时机
3. **竞态条件**: 验证所有共享状态访问都在锁保护下

## My_test 目录

`src/My_test/` 是学习者的实践区域,包含:
- `My_thread_pool.hpp/cpp` - 学习者自己实现的线程池
- 当前实现有 bug: `condition_.wait()` 的 predicate 应为 `!tasks_.empty()` 而非 `tasks_.empty()`
- 不应在生成的可执行文件中包含此目录

## 关键注意事项

1. **这是教学项目**: 代码优先考虑可读性和教学清晰度,而非极致性能
2. **所有阶段独立**: 不要引入跨阶段依赖,每个阶段都是完整实现
3. **注释是关键**: 保持详细的中文注释,这是教学材料的核心
4. **示例实用性**: `main.cpp` 中的示例应能直观展示该阶段新增概念

## 辅助命令

```bash
# 编译所有阶段
cd build && make

# 运行特定阶段
./step1_basic_threadpool   # 基础版本
./step2_with_future        # 支持返回值
./step3_advanced          # 高级特性
./final_demo              # 完整示例

# 清理重新编译
cd build && make clean && make
```
