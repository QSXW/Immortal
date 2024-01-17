/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

namespace Immortal
{

inline int CAST(AddressFamily addressFamily)
{
    return (int)addressFamily;
}

inline int CAST(Protocol protocol)
{
    switch (protocol)
    {
    case Protocol::UDP:
        return IPPROTO_UDP;

    case Protocol::TCP:
    default:
        return IPPROTO_TCP;
    }
}

inline int CAST(SocketType type)
{
    return (int)type;
}

static void ProcessSocketError(int error)
{
    const char *exception = nullptr;

    throw RuntimeException(exception);
}

struct SocketWrapper
{
    int socket;
    struct addrinfo *pAddress;
};

#define Socket_ auto &[socket, pAddress] = *(SocketWrapper *)(handle.Reserved);

Socket::Socket() :
    handle{}
{
    Socket_ socket = INVALID_SOCKET;
}

Socket::Socket(Anonymous _socket) :
    handle{}
{
    Socket_ socket = (int)(uint64_t)(_socket);
}

Socket::~Socket()
{
    Socket_
    if (*this)
    {
        Close();
    }
}

Socket::operator Anonymous() const
{
    Socket_
    return (void *)(uint64_t)socket;
}

Socket::operator bool() const
{
    Socket_
    return !(socket == INVALID_SOCKET);
}

void Socket::Open(AddressFamily addressFamily, SocketType socketType, Protocol protocol)
{
    Socket_
    if (socket > 0)
    {
        Close();
    }

    socket = ::socket(CAST(addressFamily), CAST(socketType), CAST(protocol));
}

void Socket::Bind(const std::string &ip, int port, AddressFamily addressFamily, SocketType socketType, Protocol protocol)
{
    Socket_ Open(addressFamily, socketType, protocol);

    if (socket == INVALID_SOCKET)
    {
        return;
    }

    struct addrinfo hints{};
    hints.ai_family   = CAST(addressFamily);
    hints.ai_socktype = CAST(socketType);
    hints.ai_protocol = CAST(protocol);

    int ret = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &pAddress);
    if (ret < 0)
    {
        throw SocketExecption("Get Address Error");
    }

    ret = bind(socket, pAddress->ai_addr, pAddress->ai_addrlen);
    if (ret == SOCKET_ERROR)
    {
        throw SocketExecption("Bind Error");
    }
}

void Socket::Listen(int backlog)
{
    Socket_
    int ret = listen(socket, backlog);
    if (ret == SOCKET_ERROR)
    {
        throw SocketExecption("Listen Error");
    }
}

void Socket::Connect(const std::string &ip, int port, AddressFamily addressFamily, SocketType socketType, Protocol protocol)
{
    Socket_ Open(addressFamily, socketType, protocol);

    if (socket == INVALID_SOCKET)
    {
        return;
    }

    struct addrinfo hints{};
    hints.ai_family   = CAST(addressFamily);
    hints.ai_socktype = CAST(socketType);
    hints.ai_protocol = CAST(protocol);

    if (getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &pAddress) == SOCKET_ERROR)
    {
        throw SocketExecption("Get Address Error");
    }

    if (connect(socket, pAddress->ai_addr, pAddress->ai_addrlen) == SOCKET_ERROR)
    {
        throw SocketExecption("Connect Error");
    }
}

int Socket::ShutDown(SocketOperation operation)
{
    Socket_
    return shutdown(socket, int(operation));
}

void Socket::Close()
{
    Socket_ close(socket);

    if (pAddress)
    {
        freeaddrinfo(pAddress);
        pAddress = nullptr;
    }

    socket = INVALID_SOCKET;
}

Socket *Socket::Accept()
{
    Socket_  sockaddr_in6 address{};
    socklen_t length = sizeof(sockaddr_in);

    if (pAddress->ai_family == int(AddressFamily::IPV6))
    {
        length = sizeof(sockaddr_in6);
    }

    int acceptSocket = accept(socket, (struct sockaddr *)&address, &length);
    return acceptSocket != INVALID_SOCKET ? new Socket((void *)(uint64_t)(acceptSocket)) : nullptr;
}

int Socket::Send(const char *buffer, int size)
{
    Socket_
    return write(socket, buffer, size);
}

int Socket::Receive(char*buffer, int size)
{
    Socket_
    return read(socket, buffer, size);
}

std::string Socket::GetIpString() const
{
    Socket_ char str[48] = {};

    if (pAddress->ai_family == int(AddressFamily::IPV6))
    {
        sockaddr_in6 *addr = (sockaddr_in6 *)pAddress->ai_addr;
        inet_ntop(AF_INET6, &addr->sin6_addr, str, SL_ARRAY_LENGTH(str));
    }
    else
    {
        sockaddr_in *addr = (sockaddr_in *)pAddress->ai_addr;
        inet_ntop(AF_INET, &addr->sin_addr, str, SL_ARRAY_LENGTH(str));
    }

    return str;
}

}
