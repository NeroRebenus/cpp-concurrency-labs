#pragma once

#include <atomic>
#include <vector>

template <class T>
class MPMCBoundedQueue {
public:
    explicit MPMCBoundedQueue(int size) : queue_(size) {
        max_size_ = size;
        end_ = 0;
        begin_ = 0;
        for (int i = 0; i < size; ++i) {
            queue_[i].second = i;
        }
    }

    bool Enqueue(const T& value) {
        int cur_pos = end_.load();
        while (true) {
            if (queue_[cur_pos % max_size_].second == cur_pos &&
                end_.compare_exchange_weak(cur_pos, cur_pos + 1)) {
                queue_[cur_pos % max_size_] = std::make_pair(value, cur_pos + 1);
                return true;
            }
            if (queue_[cur_pos % max_size_].second < cur_pos) {
                return false;
            }
            cur_pos = end_.load();
        }
    }

    bool Dequeue(T& data) {
        int cur_pos = begin_.load();
        while (true) {
            if (queue_[cur_pos % max_size_].second == cur_pos + 1 &&
                begin_.compare_exchange_weak(cur_pos, cur_pos + 1)) {
                data = std::move(queue_[cur_pos % max_size_].first);
                queue_[cur_pos % max_size_].second = cur_pos + queue_.size();
                return true;
            }
            if (queue_[cur_pos % max_size_].second < cur_pos + 1) {
                return false;
            }
            cur_pos = begin_.load();
        }
    }

private:
    std::vector<std::pair<T, std::atomic<int>>> queue_;
    int max_size_ = 0;
    std::atomic<int> begin_;
    std::atomic<int> end_;
};
