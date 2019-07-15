
#include "trampoline.h"

#include "Windows.h"

extern "C" uint8_t   generic_ret_trampoline;
extern "C" uint8_t   generic_trampoline;
extern "C" size_t    generic_ret_trampoline_size;
extern "C" size_t    generic_trampoline_size;

/**
 * Helper function allcation a new page for the trampoline and populating it with the current shellcode. Used for pre hooks only.
 * \param trampoline A pointer to the page address, output parameter
 * \return true on success, false on failure
 */
bool AllocTrampoline(HTPReturnTrampoline** trampoline)
{
    *trampoline = (HTPReturnTrampoline*)VirtualAlloc(NULL, generic_ret_trampoline_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if(*trampoline == NULL)
    {
        return false;
    }
    // Copying the trampoline code to the newly allocated memory.
    // Build trampoline
    memcpy(*trampoline, &generic_ret_trampoline, generic_ret_trampoline_size);
    return true;
}

/**
 * Helper function allcation a new page for the trampoline and populating it with the current shellcode. Used for pre-post hooks.
 * \param trampoline A pointer to the page address, output parameter
 * \return true on success, false on failure
 */
bool AllocTrampoline(HTPTrampoline** trampoline)
{
    *trampoline = (HTPTrampoline*)VirtualAlloc(NULL, generic_trampoline_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if(*trampoline == NULL)
    {
        return false;
    }
    // Copying the trampoline code to the newly allocated memory.
    // Build trampoline
    memcpy(*trampoline, &generic_trampoline, generic_trampoline_size);
    return true;
}

/**
 * Helper to free trampoline memory. Size of a trampoline being specific,
 * and page allocation and size could differ from a system to another it
 * is more flexible to overload these.
 * \param trampoline Address of the trampoline to be freed.
 * \return true on success, false on failure
 */
bool FreeTrampoline(HTPTrampoline* trampoline)
{
    return VirtualFree(trampoline, generic_trampoline_size, MEM_RELEASE);
}

/**
 * Helper to free trampoline memory. Size of a trampoline being specific,
 * and page allocation and size could differ from a system to another it
 * is more flexible to overload these.
 * \param trampoline Address of the trampoline to be freed.
 * \return true on success, false on failure
 */
bool FreeTrampoline(HTPReturnTrampoline* trampoline)
{
    return VirtualFree(trampoline, generic_ret_trampoline_size, MEM_RELEASE);
}