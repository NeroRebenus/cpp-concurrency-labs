# MPSC Lock-Free Stack

`MPSCStack<T>` - lock-free стек типа Multiple Producer / Single Consumer:

- `Push` можно вызывать из многих потоков параллельно.
- `Pop`/`DequeueAll` должны вызываться только одним потребителем (не concurrently).

## Интерфейс

- `void Push(const T& value)`
- `std::optional<T> Pop()`
- `template <class F> void DequeueAll(const F& cb)`

## Идея

- Односвязный список узлов `Node{T value, Node* next}`.
- Единственное поле синхронизации: `std::atomic<Node*> head_`.
- `Push` реализован через CAS-цикл (`compare_exchange_weak`).
- Потребитель может забрать весь список разом через `head_.exchange(nullptr)`.
