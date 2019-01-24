// Loader to load various types of modules

#include "htp.h"
#include "module.h"

#include "Windows.h"

// TODO: Add manual loader

// Generic loading using LoadLibrary, do not use with some security solutions
// As it can be easy to monitor.
bool LoadModule(HTPHandle* handle, const char* module_path)
{
    // TODO: Check if path exists.
    // TODO: Handle current directory
    HMODULE module_handle = LoadLibraryA(module_path);
    // TODO: Handle error
    if(module_handle != nullptr)
    {
        Module* new_module = new Module;
        new_module->handle = module_handle;
        new_module->base_address = (uintptr_t)module_handle;
        strncpy_s(new_module->module_path, MAX_PATH, module_path, strlen(module_path));
        handle->module_list.push_back(new_module);
    }
    return true;
}

bool UnloadModule(HTPHandle* handle, const char* module_path)
{
    bool found = false;
    std::list<Module*>::iterator it;

    if(module_path == nullptr)
    {
        DBGMSG("Invalid module name\n");
        return false;
    }
    // Finding specified module
    for(it = handle->module_list.begin();
        it != handle->module_list.end() && !found;
        it++)
    {
        char* current_module_name = (*it)->module_path;
        if(strncmp(current_module_name, module_path, strlen(current_module_name)))
        {
            found = true;
            FreeLibrary((*it)->handle);
            delete (*it);
            handle->module_list.erase(it);
        }
    }
    return found;
}