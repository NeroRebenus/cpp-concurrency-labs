#pragma once

#include <atomic>

struct RWSpinLock {
    std::atomic<int> bits_flag_ = 0;
    void LockRead() {
        while (true) {
            int read_count = bits_flag_.load();
            int check_write = 1;
            if (!bits_flag_.compare_exchange_strong(check_write, 1) &&
                bits_flag_.compare_exchange_strong(read_count, read_count + 2)) {
                break;
            }
        }
    }

    void UnlockRead() {
        bits_flag_ -= 2;
    }

    void LockWrite() {
        while (true) {
            int check = 0;
            if (bits_flag_.compare_exchange_strong(check, 1)) {
                break;
            }
        }
    }

    void UnlockWrite() {
        bits_flag_ = 0;
    }
};
