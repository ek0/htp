#ifndef _LOCK_H_
#define _LOCK_H_

#include <cstdint>

struct SimpleLock
{
    volatile uint8_t lock;
};

struct RecursiveLock
{
    uint32_t recursive_lock_count;
    volatile uint32_t lock_tid;
};

// Simple lock
void LockInit(SimpleLock *lock);
void Lock(SimpleLock *slock);
void Unlock(SimpleLock *slock);

// Recursive lock
void LockInit(RecursiveLock *lock);
void Lock(RecursiveLock *lock);
void Unlock(RecursiveLock *lock);

#endif