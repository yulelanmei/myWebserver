#include<stdlib.h>
#include<thread>
#include<mutex>                 //  互斥量
#include<condition_variable>    //  信号量
#include<queue>
// #include<lock.h>

// 线程安全的队列
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> q_queue;  //  维护核心队列
    std::mutex q_mutex;     //  访问互斥锁
    
public:
    ThreadSafeQueue() {}
    ~ThreadSafeQueue() {}

    bool empty() {
        std::unique_lock<std::mutex> lock(q_mutex); // unique_lock加锁，作用域结束析构时自动解锁
        return q_queue.empty();
    }

    int size() {
       std::unique_lock<std::mutex> lock(q_mutex);
       return q_queue.size(); 
    }

    void push(T &val) {
        std::unique_lock<std::mutex> lock(q_mutex);
        q_queue.push(val);
    }

    bool pop() {
        std::unique_lock<std::mutex> lock(q_mutex);
        if(q_queue.empty()) return false;
        q_queue.pop();
        return true;
    }

    T front() {
        std::unique_lock<std::mutex> lock(q_mutex);
        if(q_queue.empty()) return nullptr;
        return q_queue.front();
    }

};