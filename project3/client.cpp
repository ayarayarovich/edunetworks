#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sstream>
#include <string_view>

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

std::string extractBodyFromHTTPString(const std::string &response) {
    return response.substr(response.find("\r\n\r\n"));
}

int main()
{
    WSAStartupGuard wsaStartupGuard{};
    if (wsaStartupGuard.Start() != 0) {
        std::cerr << "wsaStartupGuard.Start() failed" << std::endl;
        return 1;
    }

    std::cout << "Simple HTTP Client\n" << std::endl;

    addrinfo* theirAddrInfo;

    addrinfo hints{};
    std::memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    std::string desiredDomainName, desiredPath;
    int errorCode = 0;
    do {
        std::cout << "Enter desired domain name (I'm gonna download the response): ";
        std::cin >> desiredDomainName;

        std::cout << "Enter desired path: ";
        std::cin >> desiredPath;

        errorCode = getaddrinfo(desiredDomainName.c_str(), "http", &hints, &theirAddrInfo);
        if (errorCode == WSAHOST_NOT_FOUND) {
            std::cerr << "DNS lookup for given host name failed. Try another one...\n" << std::endl;
            continue;
        }
        else if (errorCode != 0) {
            std::cerr << "getaddrinfo() failed: " << WSAGetLastError() << std::endl;
            return 1;
        }
    } while (errorCode != 0);
    
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

    char buffer[MAX_BUFFER_SIZE + 1];
 
    std::string request = (std::stringstream()
            << "GET " << desiredPath << " HTTP/1.1" << "\r\n"
            << "Host: " << desiredDomainName        << "\r\n"
            << "Accept: */*"                        << "\r\n"
            << "Connection: close"                  << "\r\n"
            << "\r\n").str();

    int bytesSent = send(sock, request.c_str(), (int)request.size(), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
        return 1;
    };


    std::stringstream response;

    int bytesReceived = 0;
    do {
        bytesReceived = recv(sock, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            response << buffer;
        }

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            break;
        }
    }
    while (bytesReceived > 0);

    std::string responseBody = extractBodyFromHTTPString(response.str());
    std::cout << responseBody << std::endl;

    closesocket(sock);
    freeaddrinfo(theirAddrInfo);

    system("pause");

    return 0;
}
