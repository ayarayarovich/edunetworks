#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

const int MAX_BUFFER_SIZE = 1024;

class WSAStartupGuard {
  public:
    WSAData wsaData;

    WSAStartupGuard() = default;
    int Start() {
        return WSAStartup(MAKEWORD(2,2), &wsaData);
    }

    ~WSAStartupGuard() {
        WSACleanup();
    }
};

int main()
{
    WSAStartupGuard wsaStartupGuard{};
    if (wsaStartupGuard.Start() != 0) {
        std::cerr << "wsaStartupGuard.Start() failed" << std::endl;
        return 1;
    }

    std::cout << "Client\n" << std::endl;

    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> theirAddrInfo(nullptr, &freeaddrinfo);
    addrinfo hints{};
    std::memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    if (getaddrinfo("localhost", "1234", &hints, std::out_ptr(theirAddrInfo)) != 0)
    {
        std::cerr << "getaddrinfo() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET sock = socket(theirAddrInfo->ai_family, theirAddrInfo->ai_socktype, theirAddrInfo->ai_protocol);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    if (connect(sock, theirAddrInfo->ai_addr, (int)theirAddrInfo->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "connect() failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    for (;;) {
        std::cout << "Enter the message: ";
        std::string msg;
        std::getline(std::cin, msg);
        if (msg == "quit")
        {
            break;
        }

        send(sock, msg.c_str(), msg.size(), 0);

        char buffer[MAX_BUFFER_SIZE + 1];
        int bytesReceived = recv(sock, buffer, MAX_BUFFER_SIZE, 0);
        buffer[bytesReceived] = '\0';

        std::cout << "Got message: " << buffer << '\n';
    }

    closesocket(sock);
    return 0;
}
