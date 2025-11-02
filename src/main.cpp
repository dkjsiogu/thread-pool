#include "My_test/My_thread_pool.hpp"

int main(){
    thread_pool::MyThreadPool thread_pool(4);
    
    for(size_t i=0; i<10;i++){
        thread_pool.submit([i](){
            std::cout<<"thead:"<<i<<std::endl;
        });
    }
    while(thread_pool.pending_tasks() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}