#ifndef ESP_THREAD_HELPER_H
#define ESP_THREAD_HELPER_H

#include <thread>
#include <vector>
#include <functional>
#include <string>

#include "esp_pthread.h"

class EspThreadHelper
{
public:
    void AddTask(std::function<void()> func, size_t stack_size = 2048,
        size_t prio = 5, const char* thread_name = nullptr) {
        m_cfg = esp_pthread_get_default_config();
        m_cfg.stack_size = stack_size;
        m_cfg.prio = prio;
        m_cfg.thread_name = thread_name;
        esp_pthread_set_cfg(&m_cfg);
        m_threads.push_back(std::thread(func));
    }
    void PrintInfo() {
        std::stringstream ss;
        ss << "core id: " << xPortGetCoreID()
           << ", prio: " << uxTaskPriorityGet(nullptr)
           << ", minimum free stack: " << uxTaskGetStackHighWaterMark(nullptr) << " bytes.";
        ESP_LOGI(pcTaskGetName(nullptr), "%s", ss.str().c_str());
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    ~EspThreadHelper() {
        for (auto& t : m_threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
private:
    std::vector<std::thread> m_threads;
    esp_pthread_cfg_t m_cfg;
};

#endif
