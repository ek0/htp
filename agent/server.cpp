#include "server.h"
#include "htp.h"

#include "protocol.h"

// To remove?
#pragma comment(lib, "ws2_32.lib")

/**
 * Message buffer must be NULL
 * Message buffer must be freed by the caller
 * TODO Change prototyupe?
 * TODO: Extract this logic and move server to abstract project
 */
MessageError GetMessage(HTPServer *server, HTPMessage *message)
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
                return MESSAGE_SOCKET_ERROR;

            case 0:
                DBGMSG("Client closed connection\n");
                return MESSAGE_CLIENT_DISCONNECTED;
        }
        bytes_received += result;
        DBGMSG("Bytes received: %llx\n", bytes_received);
    } while(bytes_received != sizeof(MessageHeader));
    DBGMSG("MessageHeader: magic: %x, type: %x, content size: %x\n", header.magic, header.type, header.message_size);
    if(header.magic != PROTO_MAGIC)
    {
        DBGMSG("Invalid packet format\n");
        return MESSAGE_INVALID_FORMAT;
    }
    if(header.message_size == 0)
    {
        DBGMSG("Length can't be 0\n");
        return MESSAGE_CONTENT_EMPTY;
    }
    bytes_received = 0; // resetting the bytes
    result = 0;
    // Crafting the proper message object
    message->content = new char[header.message_size];
    message->size    = header.message_size;
    message->type    = header.type;
    do
    {
        result += recv(server->client_socket, message->content, header.message_size, 0);
        switch(result)
        {
            case SOCKET_ERROR:
                DBGMSG("recv failed with error: %d\n", WSAGetLastError());
                closesocket(server->client_socket);
                return MESSAGE_SOCKET_ERROR;

            case 0:
                DBGMSG("Client closed connection\n");
                return MESSAGE_CLIENT_DISCONNECTED;
        }
        bytes_received += result;
    } while (bytes_received <= sizeof(header.message_size));
    // Successfully received the whole message, exiting
    return MESSAGE_SUCCESS;
}

// Server main loop
void ServerLoop(HTPServer* server)
{
    bool client_disconnecting = false;
    MessageError type = MESSAGE_SOCKET_ERROR;
    HTPMessage message;

    memset(&message, 0, sizeof(HTPMessage));
    // Accept a client socket
    server->client_socket = accept(server->listen_socket, nullptr, nullptr);
    if(server->client_socket == INVALID_SOCKET) {
        DBGMSG("accept failed with error: %d\n", WSAGetLastError());
        closesocket(server->listen_socket);
        WSACleanup();
    }
    server->client_connected = true;
    do {
        type = GetMessage(server, &message);
        if(type == MESSAGE_SOCKET_ERROR)
        {
            //DBGMSG("Error receiving message\n");
            return; // exiting?
        }
        // Processing messages
        if(type == MESSAGE_CLIENT_DISCONNECTED)
        {
            //DBGMSG("Client disconnecting...\n");
            client_disconnecting = true;
        }
        if(type == MESSAGE_SUCCESS)
        {
            server->OnReceive(server, &message);
        }
        // Deleting the received message
        delete message.content;
        message.content = nullptr;
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

bool SendResponse(HTPServer *server, HTPMessage *message)
{
    // TODO
    uint32_t bytes_sent = 0;
    do
    {
        bytes_sent += send(server->client_socket, message->content, message->size, 0);
        if(bytes_sent < 0)
        {
            DBGMSG("Error sending response: %d\n", WSAGetLastError);
            return false;
        }
    } while(bytes_sent != message->size);
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

bool StartServer(HTPServer* server, ServerOnReceiveCallback OnReceive)
{
    if(server == nullptr)
    {
        DBGMSG("server can't be NULL\n");
        return false;
    }
    if(!ServerInit(server))
    {
        DBGMSG("error starting the server\n");
        return false;
    }
    if(OnReceive == nullptr)
    {
        DBGMSG("callback can't be NULL\n");
        return false;
    }
    server->OnReceive = OnReceive;
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