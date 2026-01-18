#pragma once
#include <mutex>
#include <condition_variable>

class RWLock {
public:
    template <class Func>
    void Read(Func func) {
        std::unique_lock<std::mutex> lock(global_);
        while (blocked_writers_ != 0) {
            reader_.wait(lock);
        }
        ++blocked_readers_;
        lock.unlock();
        func();
        lock.lock();
        --blocked_readers_;
        if (blocked_readers_ == 0) {
            writer_.notify_one();
        } else {
            reader_.notify_all();
        }
    }

    template <class Func>
    void Write(Func func) {
        std::unique_lock<std::mutex> lock(global_);
        ++blocked_writers_;
        while (blocked_readers_ != 0) {
            writer_.wait(lock);
        }
        func();
        --blocked_writers_;
        if (blocked_writers_ != 0) {
            writer_.notify_one();
        } else {
            reader_.notify_all();
        }
    }

private:
    std::mutex global_;
    int blocked_readers_ = 0;
    int blocked_writers_ = 0;
    std::condition_variable reader_;
    std::condition_variable writer_;
};
