#include "My_test/My_thread_pool.hpp"

int add(int a,int b){
    std::cout<<a<<" + "<<b<<std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return a+b;
}
int recursion(int n){
    if(n<=2){
        return n;
    }
    return recursion(n-1)+recursion(n-2);
}
int main(){
    thread_pool::MyThreadPool thread_pool(4);
    
    auto future1 = thread_pool.submit(recursion,45);
    auto future2 = thread_pool.submit(recursion,5);
    std::cout<<future1.get()<<std::endl;
    std::cout<<future2.get()<<std::endl;
    thread_pool.wait_all();
    std::cout << "所有任务完成！" << std::endl;
    
    return 0;
}