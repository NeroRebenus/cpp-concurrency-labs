#pragma once

#include <atomic>
#include <optional>
#include <stdexcept>
#include <utility>

template <class T>
class MPSCStack {
public:
    // Push adds one element to stack top.
    //
    // Safe to call from multiple threads.
    void Push(const T& value) {
        Node* check = head_.load();
        Node* added = new Node(value, check);
        while (!head_.compare_exchange_weak(check, added)) {
            check = head_.load();
            added->next = check;
        }
    }

    // Pop removes top element from the stack.
    //
    // Not safe to call concurrently.
    std::optional<T> Pop() {
        Node* check = head_.load();
        if (!check) {
            return std::nullopt;
        }
        head_ = check->next;
        T res = check->value;
        delete check;
        return res;
    }

    // DequeuedAll Pop's all elements from the stack and calls cb() for each.
    //
    // Not safe to call concurrently with Pop()
    template <class TFn>
    void DequeueAll(const TFn& cb) {
        Node* cur = head_.exchange(nullptr);
        while (cur) {
            cb(cur->value);
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
    }

    MPSCStack() : head_(nullptr) {
    }
    ~MPSCStack() {
        Node* cur = head_.load();
        head_ = nullptr;
        while (cur) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
    }

private:
    struct Node {
        T value;
        Node* next;

        Node(T val, Node* n) : value(val), next(n) {
        }
    };
    std::atomic<Node*> head_;
};
