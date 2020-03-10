#include "htp.h"
#include "htp_internal.h"

#include "Windows.h"



struct HTPHandle* HTPInit()
{
    HMODULE module_base = NULL;
    struct HTPHandle* handle = NULL;

    handle = new HTPHandle;
    handle->image_base = (uintptr_t)GetModuleHandle(NULL);
    handle->number_of_hooks = 0;
#ifdef _M_X64
    handle->relay_pages = std::unordered_map<uintptr_t, size_t>();
    ZydisDecoderInit(&handle->decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
#else
    ZydisDecoderInit(&handle->decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_ADDRESS_WIDTH_32);
#endif
    // Hooks will be allocated during setup if needed.
    // Up to 128 entries

    // Initializing lock
    LockInit(&handle->rlock);
    return handle;
}

/**
 * Release the current HTP instance. Removing all hooks.
 * \param handle Handle the the current HTP instance.
 * \return true on success, false on failure.
 */
bool HTPClose(HTPHandle* handle)
{
    // Deallocating all arrays.
    if (RemoveAllInlineHooks(handle) == false)
    {
        DBGMSG("RemoveAllHooks failed\n");
        return false;
    }
    delete handle;
    return true;
}

/**
 * Get the process image base
 * \return process image base
 */
uintptr_t HTPGetImageBase(HTPHandle* handle)
{
    return handle->image_base;
}

size_t HTPGetNumberOfHooks(HTPHandle* handle)
{
    return handle->number_of_hooks;
}

uintptr_t HTPGetCurrentFunctionAddress(HTPHandle* handle, uintptr_t hook_address)
{
    // Careful, your hook needs to be able to access the handle ptr.
    for(auto h : handle->hook_list)
    {
        // Address of the hook should be accessible from within the hook.
        // This is not optimal, but we'll make do until refactoring.
        if(h->original_hook_address == hook_address)
        {
            return h->original_function_address;
        }
    }
    return 0;
}