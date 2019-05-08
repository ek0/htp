#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <cstdint>

enum MessageType
{
    LogMessage,
    AddHookMessage,
    RemoveHookMessage,
    ReadMemoryMessage,
    WriteMemoryMessage,
    LoadModuleMessage,
    UnloadModuleMessage,
    ClientDisconnectMessage,
    MaxMessage
};


#define GET_MESSAGE(hdr, msg_type) (msg_type *)((uint8_t*)hdr + sizeof(struct MessageHeader))

#pragma pack(push, 1)
struct MessageHeader
{
    uint32_t magic;
    enum MessageType type;
    uint32_t message_size; // Size of the incoming content
};

struct LogMessage
{
    char data[0x10000];
};

struct AddHookMessage
{
    uintptr_t virtual_address;
    uintptr_t hook_address;
};

// DLL must be loaded!!
struct RemoveHookMessage
{
    uintptr_t virtual_address;
    uintptr_t hook_address;
};

// DLL must be loaded!!
struct ReadMemoryMessage
{
    uintptr_t virtual_address;
    uint32_t  size_in_bytes;
};

struct ReadMemoryResponse
{
    size_t  buffer_size;
    uint8_t buffer[0x1000]; // TODO: Dynamic size.
};

struct WriteMemoryMessage
{
    uintptr_t virtual_address;
    size_t    size_in_bytes;
    uint8_t   data[1]; // mesage[size_in_bytes]
};

struct LoadModuleMessage
{
    char   path[260];
    size_t path_size;
};

struct UnloadModuleMessage
{
    char   path[260];
    size_t path_size;
};
#pragma pack(pop)

#endif // _PROTOCOL_H_