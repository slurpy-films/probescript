#pragma once
#include <thread>
#include <vector>
#include <mutex>

class ThreadManager {
public:
    void registerThread(std::thread t) {
        std::lock_guard<std::mutex> lock(mutex_);
        threads.emplace_back(std::move(t));
    }

    ~ThreadManager() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (std::thread& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    std::vector<std::thread> threads;
    std::mutex mutex_;
};

inline ThreadManager threadManager;