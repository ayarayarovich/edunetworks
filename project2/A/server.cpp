#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <thread>
#include <string>

const int MAX_BUFFER_SIZE = 1024;

class WSAStartupGuard
{
  public:
    WSAData wsaData;

    WSAStartupGuard() = default;

    int Start()
    {
        return WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    ~WSAStartupGuard()
    {
        WSACleanup();
    }
};

void *get_in_addr(sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((sockaddr_in *)sa)->sin_addr);
    }
    return &(((sockaddr_in6 *)sa)->sin6_addr);
}

int main()
{
    WSAStartupGuard wsaStartupGuard{};
    if (wsaStartupGuard.Start() != 0)
    {
        std::cerr << "wsaStartupGuard.Start() failed" << std::endl;
        return 1;
    }

    std::cout << "Server\n" << std::endl;
    std::cout << "Waiting for connection ..." << std::endl;

    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> myAddrInfo(nullptr, &freeaddrinfo);
    addrinfo hints{};
    std::memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo("localhost", "1234", &hints, std::out_ptr(myAddrInfo)) != 0)
    {
        std::cerr << "getaddrinfo() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET listeningSocket = socket(myAddrInfo->ai_family, myAddrInfo->ai_socktype, myAddrInfo->ai_protocol);
    if (listeningSocket == INVALID_SOCKET)
    {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    char yes;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) != 0)
    {
        std::cerr << "setsockopt() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    if (bind(listeningSocket, myAddrInfo->ai_addr, (int)myAddrInfo->ai_addrlen) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(listeningSocket);
        return 1;
    }

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(listeningSocket);
        return 1;
    }

    sockaddr_storage theirAddr{};
    int theirAddrLen = sizeof theirAddr;
    SOCKET newSocket = accept(listeningSocket, reinterpret_cast<sockaddr *>(&theirAddr), &theirAddrLen);

    if (newSocket == INVALID_SOCKET)
    {
        std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
    }
    else
    {
        char clientIP[INET6_ADDRSTRLEN];
        std::cout << "Got new connection from "
                  << inet_ntop(theirAddr.ss_family, get_in_addr((sockaddr *)&theirAddr), clientIP, INET6_ADDRSTRLEN)
                  << " on socket " << newSocket << "\n\n\n";
    }

    char buffer[MAX_BUFFER_SIZE + 1];
    for(;;) {
        int bytesReceived = recv(newSocket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesReceived <= 0)
        {
            // Socket either hung up or error, either way close it and remove from pollFds
            if (bytesReceived == 0)
            {
                std::cout << "Socket " << newSocket << " hung up" << std::endl;
            }
            else
            {
                std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
            }
            break;
        }
        else
        {
            buffer[bytesReceived] = '\0';
            std::cout << "Got message: " << buffer << std::endl;

            std::cout << "Enter message: ";
            std::string message;
            std::getline(std::cin, message);

            if (message == "quit") {
                break;
            }

            if (send(newSocket, message.c_str(), (int)message.size(), 0) == SOCKET_ERROR)
            {
                std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
            }
        }
    }

    return 0;
}
