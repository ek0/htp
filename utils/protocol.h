#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <cstdint>

// TODO: Move to server.h

typedef unsigned int MessageError;

enum MessageType
{
    MessageTypeReadMemory,
    MessageTypeWriteMemory
};

#define MESSAGE_SUCCESS             0x00000000
#define MESSAGE_INVALID_FORMAT      0x00000001
#define MESSAGE_CONTENT_EMPTY       0x00000002
#define MESSAGE_CLIENT_DISCONNECTED 0x00000003
#define MESSAGE_SOCKET_ERROR        0x00000004

//#define GET_MESSAGE(hdr, msg_type) (msg_type *)((uint8_t*)hdr + sizeof(struct MessageHeader))

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

struct WriteMemoryResponse
{
    uint32_t status;
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