#pragma once
#include <thread>
#include <vector>
#include <mutex>

class ThreadManager
{
public:
    void registerThread(std::thread t);

    ~ThreadManager();

private:
    std::vector<std::thread> threads;
    std::mutex mutex_;
};

inline ThreadManager threadManager;