# Advanced C++: Concurrency & Lock-Free Primitives

Моя реализация учебных заданий с курса Advanced C++ в ВУЗе на многопоточность. 

## Содержание

- [`timerqueue`](timerqueue/) - `TimerQueue<T>`: хранит пары (значение, момент времени), `Pop()` блокируется до ближайшего времени и возвращает соответствующий элемент.
- [`semaphore`](semaphore/) - `Semaphore`: семафор с гарантиями «честности» (FIFO-порядок выхода из `Enter`).
- [`rw-lock`](rw-lock/) - `RWLock`: readers–writer lock с предпочтением писателей.
- [`buffered-channel`](buffered-channel/) - `BufferedChannel<T>`: буферизированный канал в стиле Go (`Send/Recv/Close`).
- [`futex`](futex/) - `Mutex`: мьютекс на атомиках и `futex` (Linux-specific).
- [`rw-spinlock`](rw-spinlock/) - `RWSpinLock`: spinlock с конкурентным чтением.
- [`mpsc-stack`](mpsc-stack/) - `MPSCStack<T>`: lock-free стек (multiple producers, single consumer).
- [`fast-queue`](fast-queue/) - `MPMCBoundedQueue<T>`: bounded MPMC очередь.
- [`hazard-ptr`](hazard-ptr/) - упрощённая реализация hazard pointers.


## Навигация

В каждой папке есть собственный `README.md` с кратким описанием конкретной задачи

