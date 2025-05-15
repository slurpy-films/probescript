#include "threads.hpp"

void ThreadManager::registerThread(std::thread t) {
    std::lock_guard<std::mutex> lock(mutex_);
    threads.emplace_back(std::move(t));
}

ThreadManager::~ThreadManager() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (std::thread& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}