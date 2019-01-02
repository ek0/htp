
#include "htp.h"
#include "trampoline.h"

#include <windows.h> // TODO: Add guards if not windows?
#include <cstdio>
#include <cstdint>

#define MAX_INSTRUCTION_SIZE 15
#define MAX_RELAY_NUMBER     128

/**
 *  Resolve the JMP target address.
 *  \param runtime_address address potentially containing the jmp instruction
 *  \param target_address target address of the jmp instruction
 *  \return true if an address has been resolved
 */
bool ResolveIndirectJmp(HTPHandle* handle, uintptr_t runtime_address, uintptr_t* target_address)
{
    bool ret = false; // true if we need to resolve one more time.
    const void* data = (const void*)runtime_address;

    const ZyanUSize length = MAX_INSTRUCTION_SIZE;
    ZydisDecodedInstruction instruction;
    if(ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&handle->decoder, data, length,
       &instruction)))
    {
        if(instruction.mnemonic == ZYDIS_MNEMONIC_JMP)
        {
            if(instruction.operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY)
            {
#ifdef _M_X64
                if(instruction.operands[0].mem.base == ZYDIS_REGISTER_RIP)
                {
                    // Resolving the target address from the indirect jmp operand.
                    *target_address = *(uintptr_t*)(runtime_address + instruction.operands[0].mem.disp.value + instruction.length);
                    ret = true;
                }
#else
                // TODO: Test
                *target_address = instruction.operands[0].mem.disp.value;
                ret = true;
#endif
            }
            else
            {
                // Relative jump, computing the targeted address
                // TODO: HACK: Temporary hack for 32bits because ZydisCalcAbsoluteAddress is int64_t only
                //             but we're loosing cycles on 32bits.
                uint64_t tmp = 0;
                ZydisCalcAbsoluteAddress(&instruction, &instruction.operands[0], runtime_address, &tmp);
                *target_address = (uintptr_t)tmp;
                ret = true;
            }
        }
    }
    return ret;
}

/**
 * Get the length of the instruction at address
 * \param  target_address Address of the instruction
 * \return size of the instruction
 */
uint32_t GetInstructionLength(HTPHandle* handle, uintptr_t target_address)
{
    // Initialize decoder context
    const void* data = (const void*)target_address;

    const ZyanUSize length = MAX_INSTRUCTION_SIZE;
    ZydisDecodedInstruction instruction;
    if(ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&handle->decoder, data, length,
       &instruction)))
    {
        return instruction.length;
    }
    return 0;
}

/**
 * Get a previous FREE page from a given address.
 * \param address Current address to start the search from
 * \param min_addr Minimum address, to stop the search
 * \param granularity Granularity of the page allocation
 * \return the address of the found page. NULL on failure.
 */
uintptr_t FindPreviousPage(uintptr_t address, uintptr_t min_addr, uint32_t granularity)
{
    uintptr_t current_address = address;

    current_address -= current_address % granularity;
    current_address -= granularity;
    while (current_address >= min_addr) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((void*)current_address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0) {
            // TODO: replace printf with I/O module
            DBGMSG("VirtualQuery failed : %lu\n", GetLastError());
            break;
        }
        if (mbi.State == MEM_FREE) {
            // Found a FREE page!
            return current_address;
        }
        if ((ULONG_PTR)mbi.AllocationBase < granularity) {
            break;
        }
        // Page not FREE, trying another page.
        current_address = (uintptr_t)mbi.AllocationBase - granularity;
    }
    return NULL;
}

/**
 * Find and allocate the nearest memory page.
 * \param  start_address   Address of the page the search will start.
 * \return                 the address of the page on success, NULL otherwise.
 */
uintptr_t FindFreePage(uintptr_t start_address)
{
    SYSTEM_INFO si;
    uintptr_t min_addr;
    uintptr_t max_addr;
    uintptr_t current_address = start_address;
    void* page = NULL;

    // Retrieving the system memory information
    GetSystemInfo(&si);
    min_addr = (uintptr_t)si.lpMinimumApplicationAddress;
    max_addr = (uintptr_t)si.lpMaximumApplicationAddress;
    while (current_address >= min_addr)
    {
        // Trying to get the previous page, if not allocated.
        current_address = FindPreviousPage(current_address, min_addr, si.dwAllocationGranularity);
        if (current_address == NULL) {
            break;
        }
        // We want to allocate a full page to store our relays.
        page = VirtualAlloc((void*)current_address, si.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (page != NULL)
        {
            // Successfully allocated a page. Moving on.
            break;
        }
    }
    if (page == NULL)
    {
        // Can't apply hook, aborting.
        DBGMSG("Critical error!\n");
    }
    return (uintptr_t)page;
}

#ifdef _M_X64
HTPRelay* GetNextAvailableRelayEntry(uintptr_t relay_base)
{
    HTPRelay* relay = (HTPRelay*)relay_base;
    // Find nearest available entry.
    while(relay->allocated != 0x0)
    {
        relay += 1;
    }
    return relay;
}

/**
 *  Find the closest relay page allocated for a given address.
 */
uintptr_t FindNearestAvailableRelayPage(HTPHandle* handle, uintptr_t target_address)
{
    std::unordered_map<uintptr_t, size_t>::const_iterator it;
    for(it = handle->relay_pages.begin();
        it != handle->relay_pages.end();
        it++)
    {
        if(it->first < target_address &&
           (target_address - it->second) < 0x7FFFFFFF)
        {
            if(it->second < MAX_RELAY_NUMBER)
                return it->first;
        }
    }
    return NULL;
}

// TODO: Optimize these three function (find and increment) as we are repeating the loop.
bool IncrementRelayCount(HTPHandle* handle, uintptr_t relay_page)
{
    std::unordered_map<uintptr_t, size_t>::iterator it;
    it = handle->relay_pages.find(relay_page);
    if(it != handle->relay_pages.end())
    {
        it->second++;
        return true;
    }
    return false;
}

bool DecrementRelayCount(HTPHandle* handle, uintptr_t relay_page)
{
    std::unordered_map<uintptr_t, size_t>::iterator it;
    it = handle->relay_pages.find(relay_page);
    if(it != handle->relay_pages.end())
    {
        it->second--;
        return true;
    }
    return false;
}
#endif

/**
 *  Setup an inline hook at TargetAddress
 */
// TODO: Deload the function logic if possible?
bool SetupInlineHook(HTPHandle *handle, uintptr_t target_address, HTPHookProc hook_address)
{
    HTPTrampoline* trampoline = NULL;
    HTPHookPrologue* prologue = NULL;
    HTPHook* hook = NULL;
    size_t prologue_size = 5; // Smallest prologue we can write
    uint32_t hook_space = 0;
    DWORD dwOldProt = 0;
    uintptr_t next_address = 0;
#ifdef _M_X64
    bool using_relay = false;
    HTPRelay* relay = NULL;
#endif

    if(handle == NULL)
    {
        // Don't forget to call HTPInit
        DBGMSG("HTPHandle is NULL, don't forget to call HTPInit\n");
        return false;
    }
    if(target_address == 0)
    {
        DBGMSG("target_address is NULL\n");
        return false;
    }
    // Allocate a new hook entry, containing information about the hook.
    // This will be used during the hook removal as an example.
    hook = (HTPHook*)malloc(sizeof(HTPHook));
    if(hook == NULL)
    {
        DBGMSG("Failed to allocate the hook entry\n");
        return false;
    }
    if (AllocTrampoline(&trampoline) == false)
    {
        DBGMSG("AllocTrampoline failed : %" PRIu32 "\n", GetLastError());
        free(hook);
        return false;
    }
    // Copying trampoline code in newly allocated memory
    hook->trampoline_address = (uintptr_t)trampoline;
    // As when writing the code, the developer doesn't have any clue if
    // the jump will be resolved or not. We need to save the toplevel address.
    // i.e: The one without the redirection resolved.
    hook->original_function_address = target_address;
    // If the targeted address starts with a JMP instruction...
    while(ResolveIndirectJmp(handle, target_address, &next_address))
    {
        // ...we need to resolve the JMP until we find the appropriate space for the hook
        target_address = next_address;
    }
#if _M_X64
    // If 64bits, checking if relay page has been allocated within range.
    uintptr_t relay_base = NULL;
    relay_base = FindNearestAvailableRelayPage(handle, target_address);
    if(relay_base == NULL) // No relay found
    {
        // Trying to allocate within 2GB of the module.
        // Or the relay array is full, allocating a new page closest to target address
        relay_base = FindFreePage(target_address);
        if(relay_base != NULL)
        {
            handle->relay_pages.insert(std::make_pair(relay_base, 0));
            prologue_size = 5; // jmp rel
            relay = GetNextAvailableRelayEntry(relay_base);
            using_relay = true;
        }
        else
        {
            // Failed allocating the relay.
            // Copying the relay code at the function_entry.
            using_relay = false;
            hook->relay_address = NULL;
            relay = (HTPRelay*)target_address;
            prologue_size = 14;
        }
    }
#endif
    while(hook_space < prologue_size)
    {
        // Dissassemble instructions until we get enough space for a relative JMP
        // TODO: Check if function is big enough, we don't want to overwrite two functions
        hook_space += GetInstructionLength(handle, target_address + hook_space);
    }
    hook->number_of_opcodes = hook_space;
    // Saving the original opcodes to be overwritten
    memcpy(trampoline->saved_opcodes, (char*)target_address, hook_space);
    if(!VirtualProtect((void*)target_address, hook_space, PAGE_EXECUTE_READWRITE, &dwOldProt))
    {
        DBGMSG("VirtualProtect failed : %" PRIu32 "\n", GetLastError());
        free(hook); FreeTrampoline(trampoline);
        return false;
    }
#ifdef _M_X64
    // Setting up the hook prologue, the relay and the trampoline with the right addresses.
    memcpy(relay->prologue.opcodes, "\xFF\x25\x00\x00\x00\x00", 6);
    relay->prologue.trampoline_address = (uintptr_t)trampoline;
    if(!IncrementRelayCount(handle, relay_base))
    {
        DBGMSG("Error increment relay count for relay page: %#" PRIxPTR "\n", relay_base);
        free(hook); FreeTrampoline(trampoline);
        return false;
    }
    // If we successfully allocated a relay, use it, otherwise we already copied
    // the relay prologue to the function entry
    if(using_relay)
    {
        prologue = (HTPHookPrologue*)target_address;
        prologue->opcode = 0xe9; // jmp rel
        prologue->offset = (uint32_t)((uintptr_t)relay - target_address - 5);
        hook->relay_address = (uintptr_t)relay; // successfully using a relay
        hook->relay_page = relay_base;
        relay->allocated = 0x1;
    }
#else // _M_IX32
    prologue = (HTPHookPrologue*)target_address;
    prologue->opcode = 0xe9; // jmp rel
    prologue->offset = (uint32_t)((uintptr_t)trampoline - target_address - 5);
    trampoline->indirect_addr = &trampoline->original_function;
#endif
    trampoline->original_function = target_address + hook_space;
    trampoline->hook_address = (uintptr_t)hook_address;
    VirtualProtect((void*)target_address, hook_space, dwOldProt, &dwOldProt);
    FlushInstructionCache(GetCurrentProcess(), (void*)target_address, 0x06);
    // TODO: Move to function?
    hook->hook_address = target_address; // Patched address may differ from address provided in argument
    hook->is_active = true;
    handle->hook_list.push_back(hook);
    handle->number_of_hooks++; // Incrementing counter
    return true;
}

bool RemoveInlineHookInternal(HTPHandle* handle, HTPHook* hook_entry)
{
    DWORD dwOldProt;
#ifdef _M_X64
    HTPRelay* relay = NULL;
#endif
    uintptr_t hook_address = NULL;
    HTPTrampoline* trampoline = NULL;

    trampoline = (HTPTrampoline*)hook_entry->trampoline_address;
    hook_address = hook_entry->hook_address; // patched address may differ from the function address
    // Restoring original opcodes
    if(!VirtualProtect((void*)hook_address, hook_entry->number_of_opcodes, PAGE_EXECUTE_READWRITE, &dwOldProt))
    {
        DBGMSG("VirtualProtect failed : %" PRIu32 "\n", GetLastError());
        return false;
    }
    memcpy((char*)hook_address, trampoline->saved_opcodes, hook_entry->number_of_opcodes);
    VirtualProtect((void*)hook_address, hook_entry->number_of_opcodes, dwOldProt, &dwOldProt);
    FlushInstructionCache(GetCurrentProcess(), (void*)hook_address, 0x06);
#ifdef _M_X64
    // Freeing up the relay entry, will be reused by another hook if necessary.
    relay = (HTPRelay*)hook_entry->relay_address;
    // We don't necessarly need to clean the opcodes.
    if(relay != NULL)
    {
        relay->prologue.trampoline_address = 0x0;
        relay->allocated = 0x0;
        if(!DecrementRelayCount(handle, hook_entry->relay_page))
        {
            DBGMSG("Error decrementing relay count\n");
            return false;
        }
    }
#endif
    // Freeing the trampoline
    VirtualFree((void*)hook_entry->trampoline_address, 0x1000, MEM_RELEASE);
    hook_entry->is_active = false;
    free(hook_entry);
    // Decrementing the global hook counter
    handle->number_of_hooks--;
    return true;
}

/**
 * Completely removes the hook, the trampoline, and the relay.
 * Memory allocated is freed.
 * \param   handle          The handle to the current HTP instance.
 * \param   target_address  Address used to install the hook.
 * \return                  true on success, false otherwise.
 */
bool RemoveInlineHook(HTPHandle* handle, uintptr_t target_address)
{
    bool found = false;
    uintptr_t hook_address = NULL;
    std::list<HTPHook*>::iterator it;

    // Finding the corresponding hook
    for(it = handle->hook_list.begin();
        it != handle->hook_list.end() && !found;
        ++it)
    {
        if((*it)->original_function_address == target_address)
        {
            // Found hook entry! Deallocating and removing.
            return RemoveInlineHookInternal(handle, *it);
        }
    }
    return false;
}

/**
 * Helper function to avoid imbricating loops by calling RemoveInlineHook in a loop
 * \param handle Handle to the current HTP instance
 * \return true on success, false in failure
 */
bool RemoveAllInlineHooks(HTPHandle* handle)
{
    std::list<HTPHook*>::iterator it;

    // Finding the corresponding hook
    for(it = handle->hook_list.begin();
        it != handle->hook_list.end();
        ++it)
    {
        if(RemoveInlineHookInternal(handle, *it) == false)
        {
            DBGMSG("Removing hook failed.\n");
            return false;
        }
    }
#ifdef _M_X64
    // Deallocating relay pages as well.
    std::unordered_map<uintptr_t, size_t>::iterator relay_it;
    for(relay_it = handle->relay_pages.begin();
        relay_it != handle->relay_pages.end();
        it++)
    {
        VirtualFree((void*)relay_it->first, 0x1000, MEM_RELEASE);
    }
#endif
    return true;
}

// TODO: Move this to trampoline?
void SaveReturnAddress(HTPHandle* handle, uintptr_t return_address)
{
    Lock(&handle->rlock);
    uint32_t tid = GetCurrentThreadId();
    if(handle->return_stack.find(tid) == handle->return_stack.end())
        // Thread doesn't exist yet, inserting.
        handle->return_stack.insert(std::make_pair(tid, std::stack<uintptr_t>()));
    handle->return_stack.at(tid).push(return_address);
    Unlock(&handle->rlock);
}

uintptr_t RestoreReturnAddress(HTPHandle* handle)
{
    Lock(&handle->rlock);
    uint32_t tid = GetCurrentThreadId();
    uintptr_t return_address = handle->return_stack.at(tid).top();
    handle->return_stack.at(tid).pop();
    Unlock(&handle->rlock);
    return return_address;
}

/**
 * Install hooks to be executed on function entry and on function return
 * \param handle          Handle to the current HTP instance
 * \param target_address  Address of the function to hook
 * \param prehook_proc    Function to be executed on function entry
 * \param posthook_proc   Function to be executed on function return
 */
bool SetupInlineHook(HTPHandle*  handle,
                     uintptr_t   target_address,
                     HTPHookProc prehook_proc,
                     HTPHookProc posthook_proc)
{
    HTPReturnTrampoline* trampoline = NULL;
    HTPHookPrologue* prologue = NULL;
    HTPHook* hook = NULL;
    size_t prologue_size = 5;
    uint32_t hook_space = 0;
    DWORD dwOldProt = 0;
    uintptr_t next_address = 0;
#ifdef _M_X64
    bool using_relay = false;
    HTPRelay* relay = NULL;
#endif

    if(handle == NULL)
    {
        // Don't forget to call HTPInit
        DBGMSG("HTPHandle is NULL, don't forget to call HTPInit\n");
        return false;
    }
    if(target_address == 0)
    {
        DBGMSG("target_address is NULL\n");
        return false;
    }
    // Allocate a new hook entry, containing information about the hook.
    // This will be used during the hook removal as an example.
    hook = (HTPHook*)malloc(sizeof(HTPHook));
    if(hook == NULL)
    {
        DBGMSG("Failed to allocate the hook entry\n");
        return false;
    }
    if (AllocTrampoline(&trampoline) == false)
    {
        DBGMSG("AllocTrampoline failed : %" PRIu32 "\n", GetLastError());
        free(hook);
        return false;
    }
    // Copying trampoline code in newly allocated memory
    hook->trampoline_address = (uintptr_t)trampoline;
    // As when writing the code, the developer doesn't have any clue if
    // the jump will be resolved or not. We need to save the toplevel address.
    // i.e: The one without the redirection resolved.
    hook->original_function_address = target_address;
    // If the targeted address starts with a JMP instruction...
    while(ResolveIndirectJmp(handle, target_address, &next_address))
    {
        // ...we need to resolve the JMP until we find the appropriate space for the hook
        target_address = next_address;
    }
#if _M_X64
    // If 64bits, checking if relay page has been allocated within range.
    uintptr_t relay_base = NULL;
    relay_base = FindNearestAvailableRelayPage(handle, target_address);
    if(relay_base == NULL) // No relay found
    {
        // Trying to allocate within 2GB of the module.
        // Or the relay array is full, allocating a new page closest to target address
        relay_base = FindFreePage(target_address);
        if(relay_base != NULL)
        {
            handle->relay_pages.insert(std::make_pair(relay_base, 0));
            prologue_size = 5; // jmp rel
            relay = GetNextAvailableRelayEntry(relay_base);
            using_relay = true;
        }
        else
        {
            // Failed allocating the relay.
            // Copying the relay code at the function_entry.
            using_relay = false;
            hook->relay_address = NULL;
            relay = (HTPRelay*)target_address;
            prologue_size = 14;
        }
    }
#endif
    while(hook_space < prologue_size)
    {
        // Dissassemble instructions until we get enough space for a relative JMP
        // TODO: Check if function is big enough, we don't want to overwrite two functions
        hook_space += GetInstructionLength(handle, target_address + hook_space);
    }
    hook->number_of_opcodes = hook_space;
    // Saving the original opcodes to be overwritten
    memcpy(trampoline->saved_opcodes, (char*)target_address, hook_space);
    if(!VirtualProtect((void*)target_address, hook_space, PAGE_EXECUTE_READWRITE, &dwOldProt))
    {
        DBGMSG("VirtualProtect failed : %" PRIu32 "\n", GetLastError());
        free(hook); FreeTrampoline(trampoline);
        return false;
    }
#ifdef _M_X64
    // Setting up the hook prologue, the relay and the trampoline with the right addresses.
    memcpy(relay->prologue.opcodes, "\xFF\x25\x00\x00\x00\x00", 6);
    relay->prologue.trampoline_address = (uintptr_t)trampoline;
    if(!IncrementRelayCount(handle, relay_base))
    {
        DBGMSG("Error increment relay count for relay page: %#" PRIxPTR "\n", relay_base);
        free(hook); FreeTrampoline(trampoline);
        return false;
    }
    // If we successfully allocated a relay, use it, otherwise we already copied
    // the relay prologue to the function entry
    if(using_relay)
    {
        prologue = (HTPHookPrologue*)target_address;
        prologue->opcode = 0xe9; // jmp rel
        prologue->offset = (uint32_t)((uintptr_t)relay - target_address - 5);
        hook->relay_address = (uintptr_t)relay; // successfully using a relay
        hook->relay_page = relay_base;
        relay->allocated = 0x1;
    }
#else // _M_IX32
    prologue = (HTPHookPrologue*)target_address;
    prologue->opcode = 0xe9; // jmp rel
    prologue->offset = (uint32_t)((uintptr_t)trampoline - target_address - 5);
    trampoline->indirect_addr = &trampoline->original_function_address;
#endif
    trampoline->original_function_address = target_address + hook_space;
    trampoline->pre_hook_proc = prehook_proc;
    trampoline->post_hook_proc = posthook_proc;
    trampoline->restore_return_addres_proc = (uintptr_t)RestoreReturnAddress;
    trampoline->save_return_address_proc = (uintptr_t)SaveReturnAddress;
    trampoline->handle = handle;
    VirtualProtect((void*)target_address, hook->number_of_opcodes, dwOldProt, &dwOldProt);
    FlushInstructionCache(GetCurrentProcess(), (void*)target_address, 0x06);
    hook->hook_address = target_address; // Patched address may differ from address provided in argument
    hook->is_active = true;
    handle->hook_list.push_back(hook);
    handle->number_of_hooks++; // Incrementing counter
    return true;
}

/**
 * Wrappers around SetupInlineHook to hook target_proc residing in loaded module_name.
 */
bool SetupInlineHook(HTPHandle* handle, char* module_name, char* proc_name, HTPHookProc hook_proc)
{
    HMODULE module_handle = NULL;
    uintptr_t target_proc = NULL;
    if(module_name == NULL || proc_name == NULL || hook_proc == NULL || handle == NULL)
    {
        DBGMSG("Invalid parameter\n");
        return false;
    }
    module_handle = GetModuleHandleA(module_name);
    if(module_handle == NULL)
    {
        DBGMSG("Failed getting the module: %s, error: %" PRIu32 "\n", module_name, GetLastError());
        return false;
    }
    target_proc = (uintptr_t)GetProcAddress(module_handle, proc_name);
    if(target_proc == NULL)
    {
        DBGMSG("Failed getting function: %s, error: %" PRIu32 "\n", proc_name, GetLastError());
        return false;
    }
    return SetupInlineHook(handle, target_proc, hook_proc);
}

/**
 * Wrappers around SetupInlineHook to hook target_proc residing in loaded module_name.
 */
bool SetupInlineHook(HTPHandle* handle, char* module_name, char* proc_name, HTPHookProc prehook_proc, HTPHookProc posthook_proc)
{
    HMODULE module_handle = NULL;
    uintptr_t target_proc = NULL;
    if(module_name == NULL || 
       proc_name == NULL ||
       prehook_proc == NULL ||
       posthook_proc == NULL ||
       handle == NULL)
    {
        DBGMSG("Invalid parameter\n");
        return false;
    }
    module_handle = GetModuleHandleA(module_name);
    if(module_handle == NULL)
    {
        DBGMSG("Failed getting the module: %s, error: %" PRIu32 "\n", module_name, GetLastError());
        return false;
    }
    target_proc = (uintptr_t)GetProcAddress(module_handle, proc_name);
    if(target_proc == NULL)
    {
        DBGMSG("Failed getting function: %s, error: %" PRIu32 "\n", proc_name, GetLastError());
        return false;
    }
    return SetupInlineHook(handle, target_proc, prehook_proc, posthook_proc);
}