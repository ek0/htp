#include "htp.h"
#include "protocol.h"

#include <cstdio>

int test_var = 0x42;

#define HTP_SUCCESS          0x00000000
#define HTP_MEMORY_READONLY  0x00000001
#define HTP_MEMORY_WRITEONLY 0x00000002
#define HTP_GENERIC_ERROR    0xFFFFFFFF // Very bad

void ReadMemory(HTPServer *server, ReadMemoryMessage *message)
{
    // TODO: Check if address valid
    DBGMSG("ReadMemory: %llx, %x\n", message->virtual_address, message->size_in_bytes);
    // Test for potential errors
    //     Invalid Address
    //     Access Violation
    //     ...
    //     Or send OK then send payload
    // Client need to find a way to check what's coming
    send(server->client_socket, (char*)message->virtual_address, message->size_in_bytes, 0);
}

void WriteMemory(HTPServer *server, WriteMemoryMessage *message)
{
    DBGMSG("WriteMemory @ %llx - %#llx\n", message->virtual_address, message->data);
    memcpy((void*)message->virtual_address, message->data, message->size_in_bytes);
}

void OnReceive(HTPServer *server, HTPMessage *message)
{
    DBGMSG("Message Type: %x\n", message->type);
    switch(message->type)
    {
        case MessageTypeReadMemory:
            ReadMemory(server, (ReadMemoryMessage*)(message->content));
            break;
        case MessageTypeWriteMemory:
            WriteMemory(server, (WriteMemoryMessage*)(message->content));
            break;
    }
}

int main(int argc, char **argv)
{
    HTPServer server;

    printf("Testvar: %p\n", &test_var);
    if(!StartServer(&server, OnReceive))
    {
        puts("Error starting server");
    }
    return 0;
}