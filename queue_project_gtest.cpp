#include "queue_project.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <future>

/**
 * @brief Test fixture for the Queue class
 * 
 */
template <typename T>
class QueueTest : public ::testing::Test {
protected:
    /**
     * @brief Set up test enviorment
     * 
     */
    void SetUp() override {
        queue = new Queue<T>(5);
    }

    /**
     * @brief Delete queue
     * 
     */
    void TearDown() override {
        delete queue;
    }

    Queue<T>* queue;
};

typedef ::testing::Types<int, double> TestTypes;
TYPED_TEST_SUITE(QueueTest, TestTypes);

/**
 * @brief Test that the queue initializes with the correct size.
 *
 */
TYPED_TEST(QueueTest, Initialize) {
    EXPECT_EQ(this->queue->Size(), 5);  // checks the maximum capacity of the queue, which should match the size provided during construction.
    EXPECT_EQ(this->queue->Count(), 0); // checks that the queue is initially empty.
}

/**
 * @brief Test that pushing elements increases the count correctly.
 * 
 */
TYPED_TEST(QueueTest, PushIncreasesCount) {
    EXPECT_EQ(this->queue->Count(), 0);
    this->queue->Push(1);
    this->queue->Push(2);
    EXPECT_EQ(this->queue->Count(), 2);
}

/**
 * @brief Test that popping elements decreases the count correctly.
 * 
 */
TYPED_TEST(QueueTest, PopDecreasesCount) {
    this->queue->Push(1);
    this->queue->Push(2);
    this->queue->Pop();
    EXPECT_EQ(this->queue->Count(), 1);
    this->queue->Pop();
    EXPECT_EQ(this->queue->Count(), 0);
}


/**
 * @brief Test popping blocks when the queue is empty.
 * 
 */
TYPED_TEST(QueueTest, PopBlocksWhenEmpty) {
    std::mutex mtx;
    std::condition_variable cv;
    bool consumer_ready = false;

    // Consumer thread: tries to pop a value and blocks.
    std::thread consumer([this, &mtx, &cv, &consumer_ready]() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            consumer_ready = true;
            cv.notify_one(); // Notify producer thread that consumer is ready.
        }
        TypeParam value = this->queue->Pop();
        EXPECT_EQ(value, 84);
    });

    // Wait until the consumer thread is ready.
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&consumer_ready]() { return consumer_ready; });
    }
    
    // Producer: pushes a value into the queue.
    this->queue->Push(84);

    consumer.join();
}

/**
 * @brief Test PopWithTimeout throws an exception when the queue is empty.
 * 
 */
TYPED_TEST(QueueTest, PopWithTimeoutThrowsWhenEmpty) {
    EXPECT_THROW(this->queue->PopWithTimeout(100), std::runtime_error);
}

/**
 * @brief Test PopWithTimeout returns the correct value when an element is available.
 * 
 */
TYPED_TEST(QueueTest, PopWithTimeoutReturnsValue) {

    std::mutex mtx;
    std::condition_variable cv;
    bool consumer_ready = false;

    // Consumer thread: tries to pop a value with a timeout.
    std::thread consumer([this, &mtx, &cv, &consumer_ready]() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            consumer_ready = true;
            cv.notify_one(); // Notify producer thread that consumer is ready.
        }
        TypeParam value = this->queue->PopWithTimeout(20000);
        EXPECT_EQ(value, 84);
    });

    // Wait until the consumer thread is ready.
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&consumer_ready]() { return consumer_ready; });
    }
    
    // Producer: pushes a value into the queue.
    this->queue->Push(84);

    consumer.join();
}

/**
 * @brief Test pushing when the queue is full drops the oldest element.
 * 
 */
TYPED_TEST(QueueTest, PushDropsOldestWhenFull) {
    for (int i = 1; i <= 5; ++i) {
        this->queue->Push(i);
    }

    EXPECT_EQ(this->queue->Count(), 5);
    this->queue->Push(6); // Drops 1
    EXPECT_EQ(this->queue->Count(), 5);
    EXPECT_EQ(this->queue->Pop(), 2);
}