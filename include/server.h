#ifndef _SERVER_H_
#define _SERVER_H_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>

#define PROTO_MAGIC 0x00505448

struct HTPServer;
struct HTPMessage;

typedef void (*ServerOnReceiveCallback)(HTPServer*, HTPMessage*);
typedef void (*ServerOnErrorCallback)(HTPServer*, uint32_t);

struct HTPMessage
{
    size_t    size;
    uint32_t  type;
    char     *content;
};

struct HTPServer
{
    WSADATA  data;
    SOCKET   listen_socket;
    SOCKET   client_socket;
    struct addrinfo addrs;
    bool     is_running;
    DWORD    server_tid;
    HANDLE   server_thread_handle;
    bool     client_connected;
    // Define callbacks to be executed here.
    ServerOnReceiveCallback OnReceive;
    ServerOnErrorCallback   OnError; // TODO
};

#define BUFFER_SIZE  8192
#define DEFAULT_PORT "27023"

bool StartServer(HTPServer* server, ServerOnReceiveCallback OnReceive);
bool StopServer(HTPServer* server);

#endif // _SERVER_H_