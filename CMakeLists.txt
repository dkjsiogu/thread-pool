cmake_minimum_required(VERSION 3.10)
project(ThreadPoolTutorial)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/src)

# 第一阶段：基础版本
add_executable(step1_basic_threadpool 
    src/step1_basic/main.cpp
)

# 第二阶段：支持返回值
add_executable(step2_with_future 
    src/step2_future/main.cpp
)

# 第三阶段：完整功能版本
add_executable(step3_advanced 
    src/step3_advanced/main.cpp
)

# 最终项目：完整的线程池库
add_executable(final_demo 
    src/final/main.cpp
)

# 学习者的实践项目
add_executable(my_thread_pool
    src/main.cpp
    src/My_test/My_thread_pool.cpp  # 添加实现文件
)
