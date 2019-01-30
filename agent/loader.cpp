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
    Module* new_module = new Module;
    if(ManualDllLoad(&new_module->state, module_path) == MM_OK)
    {
        // TODO: Handle error
        // Adding agent to module list
        
        new_module->handle = (HMODULE)new_module->state.dll_base_addr;
        new_module->base_address = new_module->state.dll_base_addr;
        new_module->broker_loaded = true;
        strncpy_s(new_module->module_path, MAX_PATH, module_path, strlen(module_path));
        handle->module_list.push_back(new_module);
        return true;
    }
    else
    {
        delete new_module;
        return false;
    }
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
        /* incrementing later */)
    {
        char* current_module_name = (*it)->module_path;
        DBGMSG("Looking to unload %s. At %s\n", module_path, current_module_name);
        if(!strncmp(current_module_name, module_path, strlen(current_module_name)))
        {
            found = true;
            DBGMSG("Freeing %s\n", current_module_name);
            ManualDllUnload(&(*it)->state);
            delete (*it);
            it = handle->module_list.erase(it);
        }
        else
        {
            it++;
        }
    }
    return found;
}