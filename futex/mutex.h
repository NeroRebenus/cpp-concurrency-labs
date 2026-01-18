#pragma once

#include <atomic>
#include <cstdint>

// Atomically do the following:
//    if (*(uint64_t*)addr == expected_value) {
//        sleep_on_address(addr)
//    }
void FutexWait(void *addr, uint64_t expected_value);

// Wakeup 1 thread sleeping on the given address
void FutexWakeOne(void *addr);

// Wakeup all threads sleeping on the given address
void FutexWakeAll(void *addr);

class Mutex {
public:
    void Lock() {
        int cur = 0;
        if (!status_.compare_exchange_strong(cur, 1)) {
            if (cur != 2) {
                cur = status_.exchange(2);
            }
            while (cur != 0) {
                FutexWait(&status_, 2);
                cur = status_.exchange(2);
            }
        }
    }

    void Unlock() {
        if (status_-- != 1) {
            status_ = 0;
            FutexWakeOne(&status_);
        }
    }

private:
    std::atomic<int> status_ = 0;
};
