#ifndef HTP_INTERNAL_H
#define HTP_INTERNAL_H

// TODO: When refactoring htp.h and moving to a opaque handle
// Move this somewhere else so this doesn't bleed into the
#include "lock.h"

#include "Zydis/Zydis.h"

#include <list>
#include <stack>
#include <unordered_map>

struct HTPHook
{
    uintptr_t      trampoline_address;         // trampoline address
#ifdef _M_X64
    uintptr_t      relay_page;                 // HACK: to avoid looping multiple times while removing a hook.
    uintptr_t      relay_address;              // relay stub address (64bits only);
#endif
    uintptr_t      original_function_address;  // original function address
    uintptr_t      original_hook_address;      // address specified by the user, can differ from hook address because of
                                               // how we handle the ILT for example.
    uintptr_t      hook_address;               // hooked function address, can differ from function address
    size_t         number_of_opcodes;          // size of the saved instructions
    bool           is_active;                  // hook is enabled
};


struct HTPRelayPage
{
    uintptr_t relay_array_base;
    size_t    number_of_relays;
};

// HTP API functions
struct HTPHandle
{
    uintptr_t             image_base;       // process image base
    uintptr_t             agent_base;       // DLL module base
    size_t                number_of_hooks;  // number of hooks installed
#ifdef _M_X64
    //uintptr_t             relay_base;       // base allocated for the relays
    //size_t                number_of_relays; // current number of relays set, max: 128
    std::unordered_map<uintptr_t, size_t> relay_pages;
#endif
    std::list<HTPHook*>   hook_list;        // Singly linked list containing the installed hooks
    std::unordered_map<uint32_t, std::stack<uintptr_t>> return_stack;     // Stack used to store the original return address.
    // Disassembly related components
    ZydisDecoder          decoder;
    // Process related information

    // Lock for the hooks save/restoring return address
    RecursiveLock rlock;
};

#endif