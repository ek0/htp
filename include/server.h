#ifndef _SERVER_H_
#define _SERVER_H_

#include <winsock2.h>
#include <ws2tcpip.h>

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
};

typedef void (*ServerCallback)(HTPServer*);

#define BUFFER_SIZE  8192
#define DEFAULT_PORT "27023"

bool StartServer(HTPServer* server);
bool StopServer(HTPServer* server);

#endif // _SERVER_H_