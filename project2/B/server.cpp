#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
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
    hints.ai_socktype = SOCK_DGRAM;
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

    char buffer[MAX_BUFFER_SIZE + 1];
    for(;;) {
        sockaddr_storage theirAddr{};
        int theirAddrLen = sizeof theirAddr;
        char clientIP[INET6_ADDRSTRLEN];

        int bytesReceived = recvfrom(listeningSocket, buffer, MAX_BUFFER_SIZE, 0, reinterpret_cast<sockaddr *>(&theirAddr), &theirAddrLen);
        if (bytesReceived <= 0)
        {
            std::cerr << "recvfrom() failed: " << WSAGetLastError() << std::endl;
            continue;
        }
        else
        {
            buffer[bytesReceived] = '\0';
            std::cout << "Echo to ("
                << inet_ntop(theirAddr.ss_family, get_in_addr((sockaddr *)&theirAddr), clientIP, INET6_ADDRSTRLEN)
                << "): " << buffer << std::endl;

            if (sendto(listeningSocket, buffer, bytesReceived, 0, reinterpret_cast<sockaddr *>(&theirAddr), theirAddrLen) == SOCKET_ERROR)
            {
                std::cerr << "sendto() failed: " << WSAGetLastError() << std::endl;
            }
        }
    }

    return 0;
}
