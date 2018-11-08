#include "htp.h"

#include <cassert>
#include <cstdio>

#define NBDEBUG

uint64_t TargetFunction1(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

uint64_t TargetFunction0(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return TargetFunction1(arg + arg);
}

uint64_t TargetFunction2(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

uint64_t TargetFunction3(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

uint64_t TargetFunction4(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

uint64_t TargetFunction5(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

uint64_t TargetFunction6(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

uint64_t TargetFunction7(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

uint64_t TargetFunction8(uint64_t arg)
{
    printf("arg: %llx\n", arg);
    return arg + arg;
}

void NewFunction(HTPContext* ctx)
{
#ifdef _M_X64
    ctx->rcx = 4;
#elif defined(_M_IX86)
    (*(uint32_t*)(ctx->esp + 0x04)) = 4; // TODO: UGLY
#endif
}

void PostHook(HTPContext* ctx)
{
#ifdef _M_X64
    printf("RAX: %llx\n", ctx->rax);
    ctx->rax = 10;
#elif defined(_M_IX86)
    printf("EAX: %lx\n", ctx->eax);
    ctx->eax = 10;
#endif
}

int main(int argc, char** argv)
{
    HTPHandle handle;
    HTPInit(&handle);
    assert(TargetFunction0(2) == 8);
    assert(TargetFunction1(2) == 4);
    assert(TargetFunction2(2) == 4);
    assert(TargetFunction3(2) == 4);
    assert(TargetFunction4(2) == 4);
    assert(TargetFunction5(2) == 4);
    assert(TargetFunction6(2) == 4);
    assert(TargetFunction7(2) == 4);
    assert(TargetFunction8(2) == 4);
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction0, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction1, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction2, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction3, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction4, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction5, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction6, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction7, NewFunction));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction8, NewFunction));
    assert(TargetFunction0(2) == 8);
    assert(TargetFunction1(2) == 8);
    assert(TargetFunction2(2) == 8);
    assert(TargetFunction3(2) == 8);
    assert(TargetFunction4(2) == 8);
    assert(TargetFunction5(2) == 8);
    assert(TargetFunction6(2) == 8);
    assert(TargetFunction7(2) == 8);
    assert(TargetFunction8(2) == 8);
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction0));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction1));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction2));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction3));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction4));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction5));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction6));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction7));
    assert(RemoveInlineHook(&handle, (uintptr_t)TargetFunction8));
    assert(TargetFunction0(2) == 8);
    assert(TargetFunction1(2) == 4);
    assert(TargetFunction2(2) == 4);
    assert(TargetFunction3(2) == 4);
    assert(TargetFunction4(2) == 4);
    assert(TargetFunction5(2) == 4);
    assert(TargetFunction6(2) == 4);
    assert(TargetFunction7(2) == 4);
    assert(TargetFunction8(2) == 4);

    puts("PostHook");
    assert(TargetFunction0(2) == 8);
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction0, NewFunction, PostHook));
    assert(SetupInlineHook(&handle, (uintptr_t)TargetFunction1, NewFunction, PostHook));
    for(size_t i = 0; i < 10000; ++i)
        assert(TargetFunction0(4) == 10);
    return 0;
}