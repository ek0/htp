#include "server.h"
#include "htp.h"

// To remove?
#pragma comment(lib, "ws2_32.lib")

// Server main loop
void ServerLoop(HTPServer* server)
{
    int    result = 0;
    int    send_result = 0;
    int    recv_length = BUFFER_SIZE;
    char   recv_buffer[BUFFER_SIZE];

    // Accept a client socket
    server->client_socket = accept(server->listen_socket, nullptr, nullptr);
    if(server->client_socket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(server->listen_socket);
        WSACleanup();
    }
    server->client_connected = true;
    do {
        result = recv(server->client_socket, recv_buffer, recv_length, 0);
        if(result > 0)
        {
            DBGMSG("Bytes received: %d\n", result);
            // Echo the buffer back to the sender
            send_result = send(server->client_socket, recv_buffer, result, 0);
            if (send_result == SOCKET_ERROR) {
                DBGMSG("send failed with error: %d\n", WSAGetLastError());
                closesocket(server->client_socket);
                closesocket(server->listen_socket);
                WSACleanup();
            }
            DBGMSG("Bytes sent: %d\n", send_result);
        }
        else if(result == 0)
        {
            DBGMSG("Connection closing...\n");
        }
        else
        {
            DBGMSG("recv failed with error: %d\n", WSAGetLastError());
            closesocket(server->client_socket);
            closesocket(server->listen_socket);
            WSACleanup();
        }
    } while(result > 0);
}

// Start the server thread
bool ServerInit(HTPServer* server)
{
    int result = 0;
    struct addrinfo* result_addr = NULL;

    memset(server, 0, sizeof(struct HTPServer));
    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2,2), &server->data);
    if(result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        return false;
    }
    memset(&server->addrs, 0, sizeof(struct addrinfo));
    server->addrs.ai_family = AF_INET;
    server->addrs.ai_socktype = SOCK_STREAM;
    server->addrs.ai_protocol = IPPROTO_TCP;
    server->addrs.ai_flags = AI_PASSIVE;
    // Resolve the server address and port
    result = getaddrinfo(NULL, DEFAULT_PORT, &server->addrs, &result_addr);
    if(result != 0) {
        printf("getaddrinfo failed with error: %d\n", result);
        WSACleanup();
        return false;
    }
    // Create a SOCKET for connecting to server
    server->listen_socket = socket(result_addr->ai_family, result_addr->ai_socktype, result_addr->ai_protocol);
    if(server->listen_socket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result_addr);
        WSACleanup();
        return false;
    }
    // Setup the TCP listening socket
    result = bind(server->listen_socket, result_addr->ai_addr, (int)result_addr->ai_addrlen);
    if(result == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result_addr);
        closesocket(server->listen_socket);
        WSACleanup();
        return false;
    }
    result = listen(server->listen_socket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(server->listen_socket);
        WSACleanup();
        return false;
    }
    // No longer need server socket
    //closesocket(server->listen_socket);
    return true;
}

bool StopServer(HTPServer* server)
{
    int result = 0;

    // Killing the thread if not terminated
    if(!TerminateThread(server->server_thread_handle, 69))
    {
        DBGMSG("error terminating the server thread\n");
    }
    // shutdown the connection since we're done
    // but only if someones connected
    if(server->client_connected)
    {
        result = shutdown(server->client_socket, SD_SEND);
        if(result == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(server->client_socket);
            closesocket(server->listen_socket);
            WSACleanup();
            return false;
        }
    }
    // cleanup
    closesocket(server->listen_socket);
    closesocket(server->client_socket);
    WSACleanup();
    return true;
}

bool StartServer(HTPServer* server)
{
    if(!ServerInit(server))
    {
        DBGMSG("error starting the server\n");
        return false;
    }
    // Starting the server loop in a new thread
    server->server_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerLoop, server, 0, &server->server_tid);
    return true;
}