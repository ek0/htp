#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <cstdint>

#define MESSAGE_MAX_BUFFER_SIZE 8192

enum MessageType
{
    Command = 0,
    Output,
    Log,
    MessageTypeMax
};

typedef uint64_t ModuleId;

//  Commands
//    Hook
//    Unhook
//    Patch     // or Write Mem
//    Unpatch?
//    Read Memory
//    StartThread
//    Kill Thread
//    Load Module
//    Unload Module

struct Message
{
    MessageType type;
    ModuleId    id;   // ID of the module to receive this message
    size_t      size;
    char        data[MESSAGE_MAX_BUFFER_SIZE];
};

struct LoadModuleMessage
{
    MessageType type;
    ModuleId    id;
    size_t      size;
    char        data[MESSAGE_MAX_BUFFER_SIZE]; // module path
};

#endif // _MESSAGE_H_