#include "htp.h"

#include <Windows.h>
#include <winsock2.h>

struct HTPServer
{
    WSADATA data;
    SOCKET listen_socket;
    SOCKET client_socket;
    struct addrinfo addrs;
    bool is_running;
};

// Server main loop
void ServerLoop(HTPServer* server_handle)
{
    int result = 0;
    int send_result = 0;

    do {
        result = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (result > 0) {
            DBGMSG("Bytes received: %d\n", result);

        // Echo the buffer back to the sender
            send_result = send( ClientSocket, recvbuf, result, 0 );
            if (send_result == SOCKET_ERROR) {
                DBGMSG("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            DBGMSG("Bytes sent: %d\n", send_result);
        }
        else if (result == 0)
            DBGMSG("Connection closing...\n");
        else  {
            DBGMSG("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    } while (result > 0);
    OnClientConnect();
    OnClientDisconnect();
}

// Start the server thread
bool StartServer()
{
    int result = 0;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    // Resolve the server address and port
    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // No longer need server socket
    closesocket(ListenSocket);
}

// Executed when a peer connects
bool OnClientConnect()
{

}

// Executed when a client disconnect
bool OnClientDisconnect()
{

}

bool StopServer()
{
    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
}