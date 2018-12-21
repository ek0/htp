
#include "lock.h"

#include <intrin.h>
#include <Windows.h>

void LockInit(SimpleLock* lock)
{
    lock->lock = 0;
}

void Unlock(SimpleLock* slock)
{
    MemoryBarrier();
    slock->lock = 0;
}

void Lock(SimpleLock* slock)
{
    while(_InterlockedCompareExchange8((volatile char*)&slock->lock, 1, 0) != 0) {
        YieldProcessor();
    }
}

void LockInit(RecursiveLock* lock)
{
    lock->lock_tid = -1;
    lock->recursive_lock_count = 0;
}

void Lock(RecursiveLock *lock)
{
    uint32_t tid = GetCurrentThreadId();

    if (lock->lock_tid == tid) {
        lock->recursive_lock_count++;
    }
    while (InterlockedCompareExchange(&lock->lock_tid, tid, -1) != -1) {
        YieldProcessor();
    }
    lock->recursive_lock_count = 1;
}

void Unlock(RecursiveLock *lock)
{
    if (--lock->recursive_lock_count == 0) {
        MemoryBarrier();
        lock->lock_tid = -1;
    }
}