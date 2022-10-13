#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
// code from: https://morestina.net/blog/1400/minimalistic-blocking-bounded-queue-for-c
// https://github.com/cameron314/concurrentqueue/blob/master/concurrentqueue.h

// this should be a multiple-producer-multiple-consumer queue, I think.
// I'm not sure if it's exception safe though.
// this exercise only requires single-producer-single-consumer queues tho
template <typename T>
class thready_queue {
    std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;

    // undelying queue with a capacity
    std::queue<T> queue;
    size_t capacity = 0;
                         
public:
    // support both boundded and unbounded
    thready_queue(size_t capacity=0): capacity(capacity) {}

    // push an rvalue into the queue
    void push(T&& item) {
        {
            // lock the shared resource queue
            std::unique_lock<std::mutex> lock(mutex);
            // block if the queue is full. when woken up push into the queue
            not_full.wait(lock, [&] {return !capacity || queue.size() < capacity; }); 
            queue.push(std::move(item));
        }
        // unique_lock frees lock
        // notify a waiting consumer that they can read
        not_empty.notify_one();
    }

    // read an lvalue
    void pop(T& item) {
        {
            // lock the shared resource queue
            std::unique_lock<std::mutex> lock(mutex);
            // block if the queue is empty. when woken up pop from the queue
            not_empty.wait(lock, [&] { return !queue.empty(); });
            item = std::move(queue.front());
            queue.pop();
        }
        // notify a waiting producer that they can write
        not_full.notify_one();
    }
};
