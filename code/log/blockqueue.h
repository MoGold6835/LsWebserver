#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <deque>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>
using namespace std;


//利用互斥/锁/条件变量实现一个支持多线程并发的阻塞队列
template <typename T>
class BlockQueue
{
public:
    explicit BlockQueue(size_t max_size = 1000);
    ~BlockQueue();
    bool empty();
    bool full();
    void push_back(const T &item);
    void push_front(const T &item);
    bool pop(T &item);
    bool pop(T &item, int timeout);
    void clear();
    T front();
    T back();
    size_t capacity();
    size_t size();

    void flush();
    void Close();

private:
    deque<T> deq_;
    mutex mtx_;
    bool isClose_;
    size_t capacity_;
    condition_variable condConsumer_;
    condition_variable condProducer_;
};

template <typename T>
BlockQueue<T>::BlockQueue(size_t maxsize) : capacity_(maxsize)
{
    assert(maxsize > 0);
    isClose_ = false;
}

template <typename T>
BlockQueue<T>::~BlockQueue()
{
    Close();
}

template<typename T>
void BlockQueue<T>::clear() {
    lock_guard<mutex> locker(mtx_);
    deq_.clear();
}


template <typename T>
void BlockQueue<T>::Close()
{
    clear();
    isClose_ = true;
    condConsumer_.notify_all();
    condProducer_.notify_all();
}

template <typename T>
bool BlockQueue<T>::empty()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.empty();
}

template <typename T>
bool BlockQueue<T>::full()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.empty();
}

template <typename T>
void BlockQueue<T>::push_back(const T &item)
{
    // 条件变量要搭配unique——lock
    unique_lock<mutex> locker(mtx_);
    while (deq_.size() >= capacity_)
    {                               // 队列已满，需要等到
        condProducer_.wait(locker); // 暂停生产，等待消费者唤醒生产条件变量
    }
    deq_.push_back(item);
    condConsumer_.notify_one(); // 唤醒消费者
}

template <typename T>
void BlockQueue<T>::push_front(const T &item)
{
    unique_lock<mutex> locker(mtx_);
    while (deq_.size() >= capacity_)
    {
        condProducer_.wait(locker); // 队列满了，需要等待
    }
    deq_.push_front(item);      // 暂停生产，等待消费者唤醒生产条件变量
    condConsumer_.notify_one(); // 唤醒消费者
}

template <typename T>
bool BlockQueue<T>::pop(T &item)
{
    unique_lock<mutex> locker(mtx_);
    while (deq_.empty())
    {
        condConsumer_.wait(locker);
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template <typename T>
bool BlockQueue<T>::pop(T &item, int timeout)
{
    unique_lock<mutex> locker(mtx_);
    while (deq_.empty())
    {
        if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)
        {
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template <typename T>
T BlockQueue<T>::front()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.front();
}

template <typename T>
T BlockQueue<T>::back()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.back();
}

template <typename T>
size_t BlockQueue<T>::capacity()
{
    lock_guard<mutex> locker(mtx_);
    return capacity_;
}
template <typename T>
size_t BlockQueue<T>::size()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.size();
}
template <typename T>
void BlockQueue<T>::flush()
{
    condConsumer_.notify_one();
}



#endif