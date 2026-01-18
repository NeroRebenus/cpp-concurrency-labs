#pragma once

#include <utility>
#include <optional>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <queue>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) : size_(size), is_closed_(false) {
    }

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        bool flag = false;
        if (queue_.size() >= size_) {
            send_waiting_.push(now);
            flag = true;
        }
        while (queue_.size() >= size_ || (send_waiting_.front() != now && flag)) {
            if (is_closed_) {
                throw std::runtime_error("zakrito prohodite zavtra");
            }
            send_.wait(lock);
        }
        if (is_closed_) {
            throw std::runtime_error("zakrito prohodite zavtra");
        }
        if (flag) {
            send_waiting_.pop();
        }
        queue_.push(value);
        receive_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
            if (is_closed_) {
                return std::nullopt;
            }
            receive_.wait(lock);
        }
        if (queue_.empty()) {
            return std::nullopt;
        }
        T result = queue_.front();
        queue_.pop();
        send_.notify_all();
        return result;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mutex_);
        is_closed_ = true;
        receive_.notify_all();
        send_.notify_all();
    }

private:
    int size_;
    bool is_closed_;
    std::mutex mutex_;
    std::condition_variable send_;
    std::condition_variable receive_;
    std::queue<T> queue_;
    std::queue<std::chrono::system_clock::time_point> send_waiting_;
};
