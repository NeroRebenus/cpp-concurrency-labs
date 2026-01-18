#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <memory>
#include <unordered_set>

thread_local std::atomic<void*> hazard_ptr{nullptr};

static const int kK = 5;

std::mutex threads_lock;
std::mutex scan_lock;

class ThreadState {
public:
    std::atomic<void*>* ptr;
};

std::unordered_set<ThreadState*> threads;

struct RetiredPtr {
    void* value;
    RetiredPtr* next;
    std::function<void()> deleter;
};

std::atomic<RetiredPtr*> free_list = nullptr;
std::atomic<int> approximate_free_list_size = 0;

void ScanFreeList() {
    approximate_free_list_size = 0;
    if (!scan_lock.try_lock()) {
        return;
    }
    RetiredPtr* retired = free_list.exchange(nullptr);

    std::vector<void*> hazard;
    {
        std::unique_lock guard(threads_lock);
        for (const auto& thread : threads) {
            if (auto ptr = thread->ptr->load()) {
                hazard.push_back(ptr);
            }
        }
    }

    std::sort(hazard.begin(), hazard.end());

    RetiredPtr* cur = retired;
    while (true) {
        if (!cur) {
            break;
        }
        RetiredPtr* next = cur->next;
        if (!std::binary_search(hazard.begin(), hazard.end(), cur->value)) {
            delete cur->value;
            delete cur;
        } else {
            cur->next = free_list.load();
            while (true) {
                if (free_list.compare_exchange_weak(cur->next, cur)) {
                    break;
                }
            }
        }
        cur = next;
    }
    scan_lock.unlock();
}

template <class T>
T* Acquire(std::atomic<T*>* ptr) {
    auto value = ptr->load();
    while (true) {
        hazard_ptr.store(value);
        auto new_value = ptr->load();
        if (new_value == value) {
            return value;
        }
        value = new_value;
    }
}

inline void Release() {
    hazard_ptr.store(nullptr);
}

template <class T>
void Retire(T* value) {
    RetiredPtr* retired = new RetiredPtr(value, free_list.load());
    while (true) {
        if (free_list.compare_exchange_weak(retired->next, retired)) {
            break;
        }
    }
    if (++approximate_free_list_size > kK) {
        ScanFreeList();
    }
}

inline void RegisterThread() {
    ThreadState* new_thread = new ThreadState(&hazard_ptr);
    std::lock_guard<std::mutex> guard(threads_lock);
    threads.insert(new_thread);
}

inline void UnregisterThread() {
    std::lock_guard<std::mutex> guard(threads_lock);
    for (auto iter = threads.begin(); iter != threads.end(); ++iter) {
        if ((*iter)->ptr == &hazard_ptr) {
            auto thread = *iter;
            threads.erase(iter);
            delete thread;
            return;
        }
    }
}
