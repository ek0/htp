#include "htp.h"
#include "protocol.h"

#include <cstdio>

int test_var = 0x42;

void ReadMemory(HTPServer *server, ReadMemoryMessage *message)
{
    ReadMemoryResponse response;
}

void WriteMemory(HTPServer *server, WriteMemoryMessage *message)
{
    WriteMemoryResponse response;
}

void OnReceive(HTPServer *server, HTPMessage *message)
{
    DBGMSG("Message Type: %x\n", message->type);
    switch(message->type)
    {
        case MessageTypeReadMemory:
            ReadMemory(server, (ReadMemoryMessage*)message);
            break;
        case MessageTypeWriteMemory:
            WriteMemory(server, (WriteMemoryMessage*)message);
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