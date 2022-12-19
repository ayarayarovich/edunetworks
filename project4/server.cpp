#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <thread>

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
    std::cout << "Waiting for connections ..." << std::endl;

    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> myAddrInfo(nullptr, &freeaddrinfo);
    addrinfo hints{};
    std::memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
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

    unsigned long nonBlocking = 1;
    if (ioctlsocket(listeningSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
        std::cerr << "ioctlsocket() failed: " << WSAGetLastError() << std::endl;
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

    std::thread ioThread([listeningSocket]() {
        std::string command;
        do
        {
            std::cin >> command;
        } while (command != "quit");
        closesocket(listeningSocket);
        });

    std::vector<pollfd> pollFds{
            {
                    listeningSocket,
                    POLLIN,
                    0
            }
    };

    while (!pollFds.empty())
    {
        int activeSocketsCount = WSAPoll(pollFds.data(), pollFds.size(), -1);
        if (activeSocketsCount == SOCKET_ERROR)
        {
            std::cerr << "WSAPoll() failed: " << WSAGetLastError() << std::endl;
            return 1;
        }

        if (activeSocketsCount > 0)
        {
            for (int i = 0; i < pollFds.size(); ++i)
            {
                if (pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                    if (pollFds[i].fd == listeningSocket) {
                        // listeningSocket was closed - quit the loop
                        for (pollfd pfd : pollFds) {
                            closesocket(pfd.fd);
                        }
                        pollFds.clear();
                        continue;
                    }

                    // Client socket disconnected
                    std::cout << "Client on socket " << pollFds[i].fd << " disconnected." << std::endl;
                    closesocket(pollFds[i].fd);
                    pollFds.erase(pollFds.begin() + i);
                    continue;
                }
                if (pollFds[i].revents & POLLIN)
                {
                    pollFds[i].revents = 0;
                    pollfd activePfd = pollFds[i];
                    if (activePfd.fd == listeningSocket)
                    {
                        // Got new client
                        sockaddr_storage theirAddr{};
                        int theirAddrLen = sizeof theirAddr;
                        SOCKET newSocket = accept(listeningSocket, reinterpret_cast<sockaddr *>(&theirAddr), &theirAddrLen);

                        if (newSocket == INVALID_SOCKET)
                        {
                            std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
                        }
                        else
                        {
                            pollFds.push_back({newSocket, POLLIN, 0});

                            char clientIP[INET6_ADDRSTRLEN];
                            std::cout << "Got new connection from "
                                      << inet_ntop(theirAddr.ss_family, get_in_addr((sockaddr *)&theirAddr), clientIP, INET6_ADDRSTRLEN)
                                      << " on socket " << newSocket << '\n';
                        }
                    }
                    else
                    {
                        // We have good message from activePdf
                        char buffer[MAX_BUFFER_SIZE + 1];
                        int bytesReceived = recv(activePfd.fd, buffer, MAX_BUFFER_SIZE, 0);
                        if (bytesReceived <= 0)
                        {
                            // Socket either hung up or error, either way close it and remove from pollFds
                            if (bytesReceived == 0)
                            {
                                std::cout << "Socket " << activePfd.fd << " hung up" << std::endl;
                            }
                            else
                            {
                                std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
                            }
                            closesocket(activePfd.fd);
                            pollFds.erase(pollFds.begin() + i);
                        }
                        else
                        {
                            buffer[bytesReceived] = '\0';
                            std::cout << "Echoing: " << buffer << std::endl;
                            if (send(activePfd.fd, buffer, bytesReceived, 0) == SOCKET_ERROR) {
                                std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }

    ioThread.join();
    return 0;
}
