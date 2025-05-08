#pragma once
#include <thread>
#include <vector>


std::vector<std::thread> threads;
void registerThread(std::thread thread) {
    threads.push_back(std::move(thread));
} 

std::vector<std::thread> getThreads() {
    return std::move(threads);
}