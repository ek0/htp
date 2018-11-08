#ifndef _HTP_H_
#define _HTP_H_

#include "htp_export_config.h"

#include "Zydis/Zydis.h"

#include <cstdint>

#if !defined(_M_X64) && !defined(_M_IX86)
    #error "Target platform not supported"
#endif

#include <list>
#include <stack>
#include <unordered_map>

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

typedef void (*HTPHookProc)(struct HTPContext*);

struct HTPHook
{
    uintptr_t      trampoline_address;         // trampoline address
#ifdef _M_X64
    uintptr_t      relay_address;              // relay stub address (64bits only);
#endif
    uintptr_t      original_function_address;  // original function address
    uintptr_t      hook_address;               // hooked function address, can differ from function address
    size_t         number_of_opcodes;          // size of the saved instructions
    bool           is_active;                  // hook is enabled
};

// HTP API functions
struct HTPHandle
{
    uintptr_t             image_base;       // process image base
    uintptr_t             agent_base;       // DLL module base
    size_t                number_of_hooks;  // number of hooks installed
#ifdef _M_X64
    uintptr_t             relay_base;       // base allocated for the relays
    size_t                number_of_relays; // current number of relays set, max: 128
#endif
    std::list<HTPHook*>   hook_list;        // Singly linked list containing the installed hooks
    std::unordered_map<uint32_t, std::stack<uintptr_t>> return_stack;     // Stack used to store the original return address.
    // Disassembly related components
    ZydisDecoder          decoder;
};

//#define GetReturnAddress(ctx) (*(uintptr_t*)(ctx->rsp))
//#define GetArg(ctx, num) (*(uintptr_t*)(ctx->rsp + (num * sizeof(uintptr_t)))

bool HTP_EXPORT HTPInit(HTPHandle* handle);
bool HTP_EXPORT HTPClose(HTPHandle* handle);
bool HTP_EXPORT SetupInlineHook(HTPHandle* handle, uintptr_t target_address, HTPHookProc hook_proc);
bool HTP_EXPORT SetupInlineHook(HTPHandle* handle, uintptr_t target_address, HTPHookProc prehook_proc, HTPHookProc posthook_proc);
bool HTP_EXPORT RemoveInlineHook(HTPHandle* handle, uintptr_t target_address);
bool HTP_EXPORT RemoveAllInlineHooks(HTPHandle* handle);

#endif _HTP_H_