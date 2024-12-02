#ifndef QUEUE_PROJ_H
#define QUEUE_PROJ_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <chrono>

/**
 * @brief Queue template class for multi thread communication.
 * 
 * @tparam T Type of elements stored in the queue.
 */
template <typename T>
class Queue {
public:
    Queue(int a_size_i);
    ~Queue();

    void Push(T a_element_t);
    T Pop();
    T PopWithTimeout(int a_milliseconds_i);
    int Count();
    int Size() const;

private:
    T* data_t_;      // Dynamically allocated array for storing queue elements.
    int max_size_i_; // Maximum number of elements the queue can hold.
    int front_i_;    // Index of the front element. Gets shifted when queue reaches maximum size. Element to be popped.
    int rear_i_;     // Index where the next element will be inserted.
    int count_i_;    // Current number of elements in the queue.

    mutable std::mutex mutex_;        
    std::condition_variable cond_var_;
    std::mutex cout_mutex_;
};

#include "queue_project.tpp"

#endif // QUEUE_PROJ_H
