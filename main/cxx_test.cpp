#include <thread>
#include <cstdio>
#include <iostream>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <mutex>
#include <esp_pthread.h>

#include "esp_log.h"


static const char *TAG = "CXX_TEST";
constexpr int PRODUCER_COUNT = 1;
constexpr int CONSUMER_COUNT = 10;
constexpr int PRODUCER_MAX_DELAY = 3;
constexpr int CONSUMER_MAX_DELAY = 15;
constexpr int TASK_QUEUE_MAX_SIZE = 100;

class Pruducer {
public:
    Pruducer(std::queue<int> &taskQueue, std::mutex &taskQueueLock, std::condition_variable &taskQueueCv)
        : taskQueue_(taskQueue), taskQueueLock_(taskQueueLock), taskQueueCv_(taskQueueCv) {
            static int id = 0;
            id_ = id++;
        }

    void produce() {
        while (true) {
            // produce task
            std::this_thread::sleep_for(std::chrono::seconds(rand() % PRODUCER_MAX_DELAY));
            int taskData = rand();
            {
                std::unique_lock<std::mutex> lock(taskQueueLock_);
                while (taskQueue_.size() >= TASK_QUEUE_MAX_SIZE) {
                    taskQueueCv_.wait(lock);
                }
                taskQueue_.push(taskData);
                ESP_LOGI(TAG, "produce data %d, id[%d]", taskData, id_);
                taskQueueCv_.notify_one();
            }
        }
    }
private:
    std::queue<int> &taskQueue_;
    std::mutex &taskQueueLock_;
    std::condition_variable &taskQueueCv_;
    int id_;
};

class Consumer {
public:
    Consumer(std::queue<int> &taskQueue, std::mutex &taskQueueLock, std::condition_variable &taskQueueCv)
        : taskQueue_(taskQueue), taskQueueLock_(taskQueueLock), taskQueueCv_(taskQueueCv) {
            static int id = 0;
            id_ = id++;
        }

    void consume() {
        while (true) {
            {
                std::unique_lock<std::mutex> lock(taskQueueLock_);
                taskQueueCv_.wait(lock, [&]{
                    return !taskQueue_.empty();
                });
                int taskData = taskQueue_.front();
                taskQueue_.pop();
                taskQueueCv_.notify_one();
                ESP_LOGI(TAG, "consume data %d, id[%d]", taskData, id_);
            }
            std::this_thread::sleep_for(std::chrono::seconds(rand() % CONSUMER_MAX_DELAY));
        }
    }
private:
    std::queue<int> &taskQueue_;
    std::mutex &taskQueueLock_;
    std::condition_variable &taskQueueCv_;
    int id_;
};

void cxxDemo()
{
    srand(time(NULL));
    std::queue<int> taskQueue;
    std::mutex taskQueueLock;
    std::condition_variable taskQueueCv;

    // Create a thread using default values that can run on any core
    auto cfg = esp_pthread_get_default_config();
    esp_pthread_set_cfg(&cfg);

    std::vector<std::thread> producerConsumerThreads;
    for (size_t i = 0; i < PRODUCER_COUNT; i++) {
        producerConsumerThreads.push_back(std::thread([&]{
            Pruducer(taskQueue, taskQueueLock, taskQueueCv).produce();
        }));
    }

    for (size_t i = 0; i < CONSUMER_COUNT; i++) {
        producerConsumerThreads.push_back(std::thread([&]{
            Consumer(taskQueue, taskQueueLock, taskQueueCv).consume();
        }));
    }

    for (auto &t : producerConsumerThreads) {
        t.join();
    }
}