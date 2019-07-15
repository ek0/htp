#ifndef _TRAMPOLINE_H_
#define _TRAMPOLINE_H_

#include "htp.h"

#include <cstdint>

#ifdef _M_X64
#pragma pack(push, 1)
struct HTPReturnTrampoline
{
    uint8_t     prehook_code[0x65];
    uint8_t     saved_opcodes[20];
    uint8_t     posthook_code[0x5e];
    HTPHookProc pre_hook_proc;
    uintptr_t   original_function_address;
    HTPHookProc post_hook_proc;
    uintptr_t   save_return_address_proc;
    uintptr_t   restore_return_addres_proc;
    HTPHandle*  handle;
};

struct HTPTrampoline
{
    char context_init[31]; // Saving registers and initializing the HTPContext
    char call_instruction[6];
    char context_cleanup[28];
    char saved_opcodes[20];
    char jmp_instruction[6];
    uintptr_t hook_address;
    uintptr_t original_function;
};

struct HTPRelayPrologue
{
    uint8_t    opcodes[6];
    uintptr_t  trampoline_address;
};

struct HTPRelay
{
    HTPRelayPrologue   prologue;                      // relay to be used.
    uint16_t           allocated;                  // 1 is occupied. 0 if not.
};

struct HTPHookPrologue
{
    uint8_t opcode; // jmp rel
    uint32_t offset;
};
#pragma pack(pop)
#else
#pragma pack(push, 1)
struct HTPTrampoline
{
    char context_init[19];
    char saved_opcodes[20];
    char opcodes[2];
    uintptr_t* indirect_addr;
    uintptr_t  hook_address;
    uintptr_t  original_function;
};

struct HTPHookPrologue
{
    uint8_t opcode; // jmp rel
    uint32_t offset;
};

struct HTPReturnTrampoline
{
    char prehook_context_init[0x28];
    char saved_opcodes[20];
    char opcodes[2];
    uintptr_t* indirect_addr;
    char        posthook_context_init[0x23];
    HTPHookProc pre_hook_proc;
    uintptr_t   original_function_address;
    HTPHookProc post_hook_proc;
    uintptr_t   save_return_address_proc;
    uintptr_t   restore_return_addres_proc;
    HTPHandle*  handle;
};
#pragma pack(pop)
#endif

bool AllocTrampoline(HTPReturnTrampoline** trampoline);
bool AllocTrampoline(HTPTrampoline** trampoline);
bool FreeTrampoline(HTPTrampoline* trampoline);
bool FreeTrampoline(HTPReturnTrampoline* trampoline);

#endif // _TRAMPOLINE_H_