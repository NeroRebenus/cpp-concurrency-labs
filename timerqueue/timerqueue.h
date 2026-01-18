#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <set>

template <class T>
auto comparator = [](std::pair<T, std::chrono::system_clock::time_point> left,
                     std::pair<T, std::chrono::system_clock::time_point> right) {
    if (left.second == right.second) {
        return left.first < right.first;
    }
    return left.second < right.second;
};

template <class T>
class TimerQueue {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

public:
    void Add(const T& item, TimePoint at) {
        std::unique_lock<std::mutex> lock(mutex_);
        pairs_.insert({item, at});
        cond_.notify_all();
    }

    T Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (true) {
            if (!pairs_.empty()) {
                if (pairs_.begin()->second <= Clock::now()) {
                    T res = pairs_.begin()->first;
                    pairs_.erase(pairs_.begin());
                    return res;
                } else {
                    cond_.wait_until(lock, pairs_.begin()->second);
                }
            } else {
                cond_.wait(lock);
            }
        }
    }

private:
    std::set<std::pair<T, TimePoint>, decltype(comparator<T>)> pairs_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
