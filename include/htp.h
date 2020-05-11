#ifndef _HTP_H_
#define _HTP_H_

#include "htp_export_config.h"

#include <cstdint>

#define __STDC_FORMAT_MACRO
#include <cinttypes> // For PRIxPTR

#if !defined(_M_X64) && !defined(_M_IX86)
    #error "Target platform not supported"
#endif

#ifdef _DEBUG
#define DBGMSG(fmt, ...) printf("%s: ", __FUNCTION__); \
                         printf(fmt, __VA_ARGS__)
#else
#define DBGMSG(fmt, ...) (void)(0)
#endif

// TODO: Update struct for floating point registers and AVX?
struct HTPContext
{
#if _M_X64
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t rsp;
#else
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
#endif
};

struct HTPHandle;

typedef void (*HTPHookProc)(struct HTPContext*);

struct HTPHandle* HTP_EXPORT HTPInit();
bool HTP_EXPORT HTPClose(HTPHandle* handle);
bool HTP_EXPORT SetupInlineHook(HTPHandle* handle, char* module_name, char* proc_name, HTPHookProc hook_proc);
bool HTP_EXPORT SetupInlineHook(HTPHandle* handle, char* module_name, char* proc_name, HTPHookProc prehook_proc, HTPHookProc posthook_proc);
bool HTP_EXPORT SetupInlineHook(HTPHandle* handle, uintptr_t target_address, HTPHookProc hook_proc);
bool HTP_EXPORT SetupInlineHook(HTPHandle* handle, uintptr_t target_address, HTPHookProc prehook_proc, HTPHookProc posthook_proc);
bool HTP_EXPORT RemoveInlineHook(HTPHandle* handle, uintptr_t target_address);
bool HTP_EXPORT RemoveAllInlineHooks(HTPHandle* handle);

/* Utility function */
uintptr_t HTP_EXPORT HTPGetImageBase(HTPHandle* handle);
size_t    HTP_EXPORT HTPGetNumberOfHooks(HTPHandle* handle);
uintptr_t HTP_EXPORT HTPGetCurrentFunctionAddress(HTPHandle* handle, uintptr_t hook_address);

#endif // _HTP_H_