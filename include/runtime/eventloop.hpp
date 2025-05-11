#pragma once
#include "values.hpp"
#include "env.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <atomic>

class EventLoop {
private:
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> running{true};
    std::thread loopThread;

public:
    EventLoop() {
        loopThread = std::thread([this]() {
            this->run();
        });
    }

    ~EventLoop() {
        stop();
        if (loopThread.joinable()) {
            loopThread.join();
        }
    }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            taskQueue.push(task);
        }
        condition.notify_one();
    }

    void run() {
        while (running) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this]() {
                    return !taskQueue.empty() || !running;
                });

                if (!running && taskQueue.empty()) {
                    return;
                }

                task = taskQueue.front();
                taskQueue.pop();
            }

            if (task) {
                task();
            }
        }
    }

    void stop() {
        running = false;
        condition.notify_all();
    }
};

class EventLoopManager {
private:
    static EventLoop* globalLoop;
    static std::once_flag initFlag;

public:
    static EventLoop* getEventLoop() {
        std::call_once(initFlag, []() {
            globalLoop = new EventLoop();
        });
        return globalLoop;
    }

    static void cleanup() {
        if (globalLoop) {
            delete globalLoop;
            globalLoop = nullptr;
        }
    }
};