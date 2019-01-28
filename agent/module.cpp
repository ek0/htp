#include "htp.h"
#include "module.h"

#include <tlhelp32.h>

bool GetProcessModules(HTPHandle* handle)
{
    Module* new_module = nullptr;
    MODULEENTRY32 module_entry;
    HANDLE module_snapshot_handle = INVALID_HANDLE_VALUE;

    // TODO: Check if list already exists.
    module_snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if(module_snapshot_handle == INVALID_HANDLE_VALUE)
    {
        DBGMSG("CreateToolhelp32Snapshot failed");
        return false;
    }
    module_entry.dwSize = sizeof(MODULEENTRY32);
    if (!Module32First(module_snapshot_handle, &module_entry)) {
        DBGMSG("Module32First failed\n");
        CloseHandle(module_snapshot_handle);
        return false;
    }
    do
    {
        if (module_entry.modBaseAddr == (PBYTE)GetModuleHandleA(NULL))
            continue;
        new_module = new Module;
        if (new_module == NULL) {
            DBGMSG("malloc failed\n");
            return false;
            //ExitProcess(42);
        }
        //new_module->th32ModuleID = module_entry.th32ModuleID;
        //new_module->th32ProcessID = module_entry.th32ProcessID;
        //new_module->GlblcntUsage = module_entry.GlblcntUsage;
        //new_module->ProccntUsage = module_entry.ProccntUsage;
        new_module->base_address = (uintptr_t)module_entry.modBaseAddr;
        new_module->module_size = module_entry.modBaseSize;
        new_module->handle = module_entry.hModule;
        new_module->broker_loaded = false;
        //memcpy(new_module->szModule, module_entry.szModule, sizeof(module_entry.szModule));
        //_strlwr_s(new_module->szModule, sizeof(new_module->szModule) - 1);
        memcpy(new_module->module_path, module_entry.szExePath, sizeof(module_entry.szExePath));
        DBGMSG("Working on %s\n", new_module->module_path);
        //mo->lExport = GetExportList((ULONG_PTR)new_module->modBaseAddr);
        handle->module_list.push_back(new_module);
    } while (Module32Next(module_snapshot_handle, &module_entry));
    return true;
}

bool CleanupModuleList(HTPHandle* handle)
{
    std::list<Module*>::iterator it;

    if(handle == nullptr)
        return false; // Invalid handle
    if(handle->module_list.empty())
        return true; // Nothing to be done
    for(it = handle->module_list.begin();
        it != handle->module_list.end();
        /*no need to increment*/)
    {
        DBGMSG("Deleting %s from module list\n", (*it)->module_path);
        delete (*it);
        it = handle->module_list.erase(it);
    }
    return true;
}