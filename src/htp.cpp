#include "htp.h"

#include "Windows.h"

bool HTPInit(HTPHandle* handle)
{
    HMODULE module_base = NULL;

    handle->image_base = (uintptr_t)GetModuleHandle(NULL);
    // avoiding passing HINSTANCE from DllMain since we don't really
    // know how the dll will be injected.
    // TODO: Move this into a parent project
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)HTPInit, // Function within the DLL address space.
                       &module_base);
    handle->agent_base = (uintptr_t)module_base;
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
    return true;
}

/**
 * Release the current HTP instance. Removing all hooks.
 * \param handle Handle the the current HTP instance.
 * \return true on success, false on failure.
 */
bool HTPClose(HTPHandle* handle)
{
    // Deallocating all arrays.
    if(RemoveAllInlineHooks(handle) == false)
    {
        DBGMSG("RemoveAllHooks failed\n");
        return false;
    }
    return true;
}