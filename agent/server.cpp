#include "server.h"
#include "htp.h"

#include "protocol.h"

// To remove?
#pragma comment(lib, "ws2_32.lib")

bool ProcessReadMemoryMessage(HTPServer *server, struct ReadMemoryMessage *message)
{
    // TODO: check for correct address
    // TODO: check for correct size
    return false;
}

bool ProcessWriteMemoryMessage(HTPServer *server, struct WriteMemoryMessage *message)
{
    // TODO: check for correct address
    // TODO: check for correct size
    return false;
}

bool ProcessLoadModuleMessage(HTPServer *server, struct LoadModuleMessage *message)
{
    // TODO: check for correct address
    // TODO: check for correct size
    return false;
}

bool ProcessUnloadModuleMessage(HTPServer *server, struct UnloadModuleMessage *message)
{
    // TODO: check for correct address
    // TODO: check for correct size
    return false;
}

// Command processing routine
bool ProcessCommand(HTPServer* server, struct MessageHeader *header, size_t buffer_size)
{
    if(header == nullptr || header->magic != 0x41424344)
    {
        DBGMSG("[+] Invalid Message...\n");
        return false;
    }
    switch(header->type)
    {
        case LogMessage:
        case AddHookMessage:
        case RemoveHookMessage:
            return false; // not implemented
        case ReadMemoryMessage:
            return ProcessReadMemoryMessage(server, GET_MESSAGE(header, struct ReadMemoryMessage));
        case WriteMemoryMessage:
            return ProcessWriteMemoryMessage(server, GET_MESSAGE(header, struct WriteMemoryMessage));
        case LoadModuleMessage:
            return ProcessLoadModuleMessage(server, GET_MESSAGE(header, struct LoadModuleMessage));
        case UnloadModuleMessage:
            return ProcessUnloadModuleMessage(server, GET_MESSAGE(header, struct UnloadModuleMessage));
        default:
            return false;
    }
}

// On new connection
void OnClientConnection(HTPServer *server)
{

}

/**
 * Message buffer must be NULL
 * Message buffer must be freed by the caller
 * TODO Change prototyupe?
 * TODO: Extract this logic and move server to abstract project
 */
MessageType GetMessage(HTPServer *server, char *message_buffer)
{
    size_t bytes_received = 0;
    int result = 0;
    MessageHeader header;

    do
    {
        // TODO: Fix nonblocking?
        // TODO: Adjust buffer for nonblocking sync
        result = recv(server->client_socket, (char*)&header, sizeof(header), 0);
        switch(result)
        {
            case SOCKET_ERROR:
                DBGMSG("recv failed with error: %d\n", WSAGetLastError());
                closesocket(server->client_socket);
                closesocket(server->listen_socket);
                WSACleanup();
                return MaxMessage;

            case 0:
                DBGMSG("Client closed connection\n");
                return ClientDisconnectMessage;
        }
        bytes_received += result;
        DBGMSG("Bytes received: %llx\n", bytes_received);
    } while(bytes_received != sizeof(MessageHeader));
    DBGMSG("MessageHeader: magic: %x, type: %x, content size: %x\n", header.magic, header.type, header.message_size);
    bytes_received = 0; // resetting the bytes
    message_buffer = new char[header.message_size];
    do
    {
        bytes_received += recv(server->client_socket, message_buffer, header.message_size, 0);
    } while (bytes_received <= sizeof(header.message_size));
    // TODO: Remove this someplace else, in a callbakc for example
    struct ReadMemoryMessage *tmp = (struct ReadMemoryMessage*)message_buffer;
    DBGMSG("Address: %llx, Size: %x\n", tmp->virtual_address, tmp->size_in_bytes);
    result = send(server->client_socket, (char*)tmp->virtual_address, tmp->size_in_bytes, 0);
    DBGMSG("Result: %x %x\n", result, WSAGetLastError());
    return header.type;
}

// Server main loop
void ServerLoop(HTPServer* server)
{
    //int    result = 0;
    //int    send_result = 0;
    //int    recv_length = BUFFER_SIZE;
    //char   recv_buffer[BUFFER_SIZE];
    //struct MessageHeader *header;
    bool client_disconnecting = false;
    MessageType type = MaxMessage;
    char *message = nullptr;

    // Accept a client socket
    server->client_socket = accept(server->listen_socket, nullptr, nullptr);
    if(server->client_socket == INVALID_SOCKET) {
        DBGMSG("accept failed with error: %d\n", WSAGetLastError());
        closesocket(server->listen_socket);
        WSACleanup();
    }
    server->client_connected = true;
    do {
        type = GetMessage(server, message);
        if(type == MaxMessage)
        {
            // this is dumb TODO
            DBGMSG("Error receiving message\n");
            return; // exiting?
        }
        // Processing messages
        if(type == ClientDisconnectMessage)
        {
            DBGMSG("Client disconnecting...\n");
            client_disconnecting = true;
        }
        delete[] message;
    } while(!client_disconnecting);
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
        DBGMSG("WSAStartup failed with error: %d\n", result);
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
        DBGMSG("getaddrinfo failed with error: %d\n", result);
        WSACleanup();
        return false;
    }
    // Create a SOCKET for connecting to server
    server->listen_socket = socket(result_addr->ai_family, result_addr->ai_socktype, result_addr->ai_protocol);
    if(server->listen_socket == INVALID_SOCKET) {
        DBGMSG("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result_addr);
        WSACleanup();
        return false;
    }
    // Setup the TCP listening socket
    result = bind(server->listen_socket, result_addr->ai_addr, (int)result_addr->ai_addrlen);
    if(result == SOCKET_ERROR) {
        DBGMSG("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result_addr);
        closesocket(server->listen_socket);
        WSACleanup();
        return false;
    }
    // No longer need server socket
    //closesocket(server->listen_socket);
    return true;
}

bool ServerListen(HTPServer *server)
{
    int result = 0;

    result = listen(server->listen_socket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        DBGMSG("listen failed with error: %d\n", WSAGetLastError());
        closesocket(server->listen_socket);
        WSACleanup();
        return false;
    }
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
            DBGMSG("shutdown failed with error: %d\n", WSAGetLastError());
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

    while(server->is_running = true)
    {
        // Wait for a connection
        ServerListen(server);
        // Starting the server loop in a new thread
        server->server_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerLoop, server, 0, &server->server_tid);
        WaitForSingleObject(server->server_thread_handle, INFINITE);
    }
    return true;
}