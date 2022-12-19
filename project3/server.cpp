#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <exception>
#include <WinSock2.h>
#include <WS2tcpip.h>

class WSAStartupGuard {
  public:
    WSAStartupGuard() = default;
    int Start() {
        WSAData wsaData{};
        return WSAStartup(MAKEWORD(2,2), &wsaData);

    }
    ~WSAStartupGuard() {
        WSACleanup();
    }
};

int main()
{

    WSAStartupGuard wsaStartupGuard;
    wsaStartupGuard.Start();

    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> myAddrInfo(nullptr, &freeaddrinfo);
    addrinfo hints{};
    std::memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;


    if (getaddrinfo("localhost", "http", &hints, std::out_ptr(myAddrInfo)) != 0) {
        std::cerr << "getaddrinfo() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET listeningSocket = socket(myAddrInfo->ai_family, myAddrInfo->ai_socktype, myAddrInfo->ai_protocol);
    if (listeningSocket == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    char yes;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) != 0) {
        std::cerr << "setsockopt() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    if (bind(listeningSocket, myAddrInfo->ai_addr, (int)myAddrInfo->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(listeningSocket);
        return 1;
    }

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(listeningSocket);
        return 1;
    }

    for (;;) {

        SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        const int MAX_CLIENT_BUFFER_SIZE = 1024;
        char buffer[MAX_CLIENT_BUFFER_SIZE];

        int bytesReceived = recv(clientSocket, buffer, MAX_CLIENT_BUFFER_SIZE, 0);

        std::stringstream response, responseBody;

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
        } else if (bytesReceived == 0) {
            std::cerr << "Connection closed..." << std::endl;
        } else if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';



            responseBody << std::ifstream("C:/Users/super/dev/http-server/index.html").rdbuf();

            response << "HTTP/1.1 200 OK\r\n"
                     << "Version: HTTP/1.1\r\n"
                     << "Content-Type: text/html; charset=utf-8\r\n"
                     << "Content-Length: " << responseBody.str().length()
                     << "\r\n\r\n"
                     << responseBody.str();

            bytesReceived = send(clientSocket, response.str().c_str(), (int)response.str().length(), 0);
            if (bytesReceived == SOCKET_ERROR) {
                std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
            }

            closesocket(clientSocket);
        }
    }

    closesocket(listeningSocket);

    return 0;
}
