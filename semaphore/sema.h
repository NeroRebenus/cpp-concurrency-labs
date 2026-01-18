#pragma once

#include <mutex>
#include <condition_variable>
#include <set>

class DefaultCallback {
public:
    void operator()(int& value) {
        --value;
    }
};

class Semaphore {
public:
    Semaphore(int count) : count_(count), timestamps_() {
    }

    void Leave() {
        std::unique_lock<std::mutex> lock(mutex_);
        ++count_;
        cv_.notify_all();
    }

    template <class Func>
    void Enter(Func callback) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        vector_mutex_.lock();
        timestamps_.insert(now);
        vector_mutex_.unlock();
        while (!count_ && *timestamps_.begin() != now) {
            cv_.wait(lock);
        }
        timestamps_.erase(timestamps_.begin());
        callback(count_);
    }

    void Enter() {
        DefaultCallback callback;
        Enter(callback);
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_ = 0;
    std::mutex vector_mutex_;
    std::set<std::chrono::system_clock::time_point> timestamps_;
};
