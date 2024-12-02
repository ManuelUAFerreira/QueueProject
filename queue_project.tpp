#include "queue_project.hpp"

//Console output switch
static bool g_verbose_mode_b = false;

/**
 * @brief Constructs a Queue with a given maximum size.
 * @param size Maximum number of elements the queue can hold.
 */
template <typename T>
Queue<T>::Queue(int a_size_i)
    : max_size_i_(a_size_i), front_i_(0), rear_i_(0), count_i_(0)
{
    if (a_size_i <= 0) {
        throw std::invalid_argument("Queue size must be greater than 0.");
    }
    data_t_ = new T[a_size_i];
}

/**
 * @brief Destructor to clean up allocated memory.
 */
template <typename T>
Queue<T>::~Queue() {
    delete[] data_t_;
}

/**
 * @brief Pushes an element into the queue. Drops the oldest element if the queue is full.
 * @param element The element to push.
 */
template <typename T>
void Queue<T>::Push(T a_element_t) {
    std::unique_lock<std::mutex> lock(mutex_);

    // If max size has been reached, drop oldest elemnt
    if (count_i_ == max_size_i_) {
        // Drop the oldest element by advancing the front.
        front_i_ = (front_i_ + 1) % max_size_i_;
        --count_i_; // discarded previous front value
    }

    data_t_[rear_i_] = a_element_t;
    rear_i_ = (rear_i_ + 1) % max_size_i_; // check if looped
    ++count_i_;
    cond_var_.notify_one(); // Notify any waiting thread.
}

/**
 * @brief Pops an element from the queue. Blocks indefinitely if the queue is empty.
 * @return The popped element.
 */
template <typename T>
T Queue<T>::Pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (g_verbose_mode_b)
    {
        std::cout <<  "Popping" << std::endl;
    }

    // Wait until an element is available.
    cond_var_.wait(lock, [this]() 
    {
        return count_i_ > 0; 
    }
    );

    // Retrieve element to pop
    T element = data_t_[front_i_];

    // Shift front index
    front_i_ = (front_i_ + 1) % max_size_i_;
    --count_i_;

    if (g_verbose_mode_b)
    {
        std::cout <<  "Popped" << std::endl;
    }

    return element;
}

/**
 * @brief Pops an element from the queue with a timeout.
 * @param a_milliseconds_i timeout value
 * @return The popped element.
 */
template <typename T>
T Queue<T>::PopWithTimeout(int a_milliseconds_i) {
    std::unique_lock<std::mutex> lock(mutex_);

    // Wait for a_milliseconds_i for an element to be available in the queue
    if (!cond_var_.wait_for(lock, std::chrono::milliseconds(a_milliseconds_i), [this]() { return count_i_ > 0; })) {
        throw std::runtime_error("Timeout: No elements available in the queue.");
    }

    // Retrieve element to pop
    T element = data_t_[front_i_];

    // Shift front index
    front_i_ = (front_i_ + 1) % max_size_i_;
    --count_i_;
    return element;
}

/**
 * @brief Returns the current number of elements stored in the queue.
 * @return The current element count.
 */
template <typename T>
int Queue<T>::Count() {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_i_;
}

/**
 * @brief Returns the maximum capacity of the queue.
 * @return The maximum number of elements the queue can hold.
 */
template <typename T>
int Queue<T>::Size() const {
    return max_size_i_;
}