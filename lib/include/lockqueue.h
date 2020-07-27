#pragma once
#include <queue>
#include <thread>
#include <mutex>    // pthread_mutex_t
#include <condition_variable>   // pthread_condition_t


// 异步写日志的日志队列，模板必须在一个文件中
template<typename T>
class LockQueue
{
public:
    // 多个worker线程都会写日志queue
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        // 当队列不为空，唤醒写日志线程
        m_condvariable.notify_one();
    }

    // 一个线程读日志queue，写日志文件
    T Pop()
    {
        // 条件变量需要和锁搭配使用
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 日志队列为空，线程进入wait状态
            m_condvariable.wait(lock);
        }
        
        T data = m_queue.front();   // data是局部变量，不能返回引用
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;  // 队列
    std::mutex m_mutex;     // 锁
    std::condition_variable m_condvariable; // 条件变量
};