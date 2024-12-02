#include <iostream>
#include <thread>
#include "queue_project.hpp"

/**
 * @brief Thread function to push value to queue (made for a specific result)
 * 
 * @param queue A reference to the thread-safe queue used for storing integers.
 * @param mtx A mutex used to protect shared variables during thread synchronization.
 * @param cv A condition variable used for inter-thread communication.
 * @param read_ready_b A boolean flag indicating whether the reader is ready to process elements.
 * @param write_ready_b A boolean flag indicating whether the writer has completed its operation.
 *
 */
void WritingThread(Queue<int>& queue, std::mutex& mtx, std::condition_variable& cv, bool& read_ready_b, bool& write_ready_b) 
{
    queue.Push(1);
    std::cout <<  "Push(1)" << std::endl;

    // ---------- Pop 1 ----------

    // Wait for the first Pop()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&read_ready_b]() { return read_ready_b; });
        read_ready_b = false;
    }

    queue.Push(2);
    std::cout <<  "Push(2)" << std::endl;

    queue.Push(3);
    std::cout <<  "Push(3)" << std::endl;

    queue.Push(4); // Drops the oldest element (2)
    std::cout <<  "Push(4)" << std::endl;

    // Notify that all 3 pushes were performed
    {
        std::unique_lock<std::mutex> lock(mtx);
        write_ready_b = true;
        cv.notify_one();
    }

    // ---------- Pop 3 4 () ----------

    // Wait for Pop 3 and 4
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&read_ready_b]() { return read_ready_b; });
        read_ready_b = false;
    }

    // Wait for Pop() to initiate first
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    queue.Push(5); // Push after the reader blocks on Pop()
    std::cout <<  "Push(5)" << std::endl;
}

/**
 * @brief Thread function to pop values from queue (made for a specific result)
 * 
 * @param queue A reference to the thread-safe queue used for storing integers.
 * @param mtx A mutex used to protect shared variables during thread synchronization.
 * @param cv A condition variable used for inter-thread communication.
 * @param read_ready_b A boolean flag indicating whether the reader is ready to process elements.
 * @param write_ready_b A boolean flag indicating whether the writer has completed its operation.
 *
 */
void ReadingThread(Queue<int>& queue, std::mutex& mtx, std::condition_variable& cv, bool& read_ready_b, bool& write_ready_b) 
{
    // Check if queue already has the first value and pop it
    if(queue.Count() > 0)
    {
        std::cout <<  "Pop() -> " + std::to_string(queue.Pop()) << std::endl; // Pop(1)
    }
    
    // Notify writer that it may continue
    {
        std::unique_lock<std::mutex> lock(mtx);
        read_ready_b = true;
        cv.notify_one(); // Notify producer thread that consumer is ready.
    }

    // ---------- PUSH 2 3 4 ----------

    // Wait for writer to push 2,3 and 4
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&write_ready_b]() { return write_ready_b; });
        write_ready_b = false;
    }

    std::cout <<  "Pop() -> " + std::to_string(queue.Pop()) << std::endl; // Pop(3)

    std::cout <<  "Pop() -> " + std::to_string(queue.Pop()) << std::endl; // Pop(4)

    // Notify writer that pops were performed
    {
        std::unique_lock<std::mutex> lock(mtx);
        read_ready_b = true;
        cv.notify_one(); // Notify producer thread that consumer is ready.
    }
    
    //Run Pop before push(5)
    std::cout <<  "Pop() -> " + std::to_string(queue.Pop()) + " // is released" << std::endl; // Pop(5)

    // ---------- PUSH 5 ----------
}

/**
 * @brief main function
 * 
 */
int main() {
    Queue<int> queue(2); // Initialize queue with size 2

    std::mutex mtx;
    std::condition_variable cv;
    bool read_ready_b = false;
    bool write_ready_b = false;

    std::thread writer(WritingThread, std::ref(queue), std::ref(mtx), std::ref(cv), std::ref(read_ready_b), std::ref(write_ready_b));
    std::thread reader(ReadingThread, std::ref(queue), std::ref(mtx), std::ref(cv), std::ref(read_ready_b), std::ref(write_ready_b));

    writer.join();
    reader.join();

    return 0;
}