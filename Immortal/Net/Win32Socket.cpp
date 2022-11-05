#include "Socket.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Ntdll.lib")

#include <Winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <Mstcpip.h>
#include <ip2string.h>

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
    switch (error)
    {
    case WSANOTINITIALISED:
        exception = "A successful WSAStartup call must occur before using this function.";
        break;
      
    case WSAENETDOWN:
        exception = "The network subsystem has failed";
        break;

    case WSAENOTSOCK:
        exception = "The descriptor is not a socket";
        break;

    case WSAEINPROGRESS:
        exception = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function";
        break;

    case WSAEINTR:
        exception = "The(blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall";
        break;

    case WSAEWOULDBLOCK:
        exception = "The socket is marked as nonblocking, but the l_onoff member of the linger structure is set to nonzeroand the l_linger member of the linger structure is set to a nonzero timeout value.";
        break;

    case WSAEAFNOSUPPORT:
        exception = "The specified address family is not supported.For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapterand device driver is not installed on the local computer";
        break;

    case WSAEMFILE:
        exception = "No more socket descriptors are available";
        break;

    case WSAEINVAL:
        exception = "An invalid argument was supplied.This error is returned if the af parameter is set to AF_UNSPECand the typeand protocol parameter are unspecified";
        break;

    case WSAEINVALIDPROVIDER:
        exception = "The service provider returned a version other than 2.2";
        break;

    case WSAEINVALIDPROCTABLE:
        exception = "The service provider returned an invalid or incomplete procedure table to the WSPStartup";
        break;

    case WSAENOBUFS:
        exception = "No buffer space is available.The socket cannot be created";
        break;

    case WSAEPROTONOSUPPORT:
        exception = "The specified protocol is not supported";
        break;

    case WSAEPROTOTYPE:
        exception = "The specified protocol is the wrong type for this socket";
        break;
        
    case WSAEPROVIDERFAILEDINIT:
        exception = "The service provider failed to initialize.This error is returned if a layered service provider(LSP) or namespace provider was improperly installed or the provider fails to operate correctly.";
        break;
            
    case WSAESOCKTNOSUPPORT:
        exception = "The specified socket type is not supported in this address family";
        break;

    default:
        exception = "Unknown socket error occurred";
        break;
    }

    throw RuntimeException(exception);    
}

struct SocketWrapper
{
    SOCKET socket;
    addrinfo *pAddress;
};

WSAData wsaData;

#define Socket_ auto &[socket, pAddress] = *(SocketWrapper *)(handle.Reserved);

Socket::Socket() :
    handle{}
{
    Socket_ socket = INVALID_SOCKET;
}

Socket::Socket(Anonymous _socket) :
    handle{}
{
    Socket_ socket = Deanonymize<SOCKET>(_socket);
}

Socket::~Socket()
{
    Socket_
    if (*this)
    {
        Close();
        WSACleanup();
    }
}

Socket::operator Anonymous() const
{
    Socket_
    return Anonymize(socket);
}

Socket::operator bool() const
{
    Socket_
    return !(socket == INVALID_SOCKET);
}

void Socket::Open(AddressFamily addressFamily, SocketType socketType, Protocol protocol)
{
    Socket_
    if (socket == INVALID_SOCKET)
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
        {
            throw RuntimeException("Failed to initiate Winsock DLL");
        }
    }
    else
    {
        Close();
    }

    socket = ::socket(CAST(addressFamily), CAST(socketType), CAST(protocol));
    if (socket == INVALID_SOCKET)
    {
        ProcessSocketError(WSAGetLastError());
    }
}

void Socket::Bind(const std::string &ip, int port, AddressFamily addressFamily, SocketType socketType, Protocol protocol)
{
    Socket_ Open(addressFamily, socketType, protocol);

    addrinfo hints{};
    hints.ai_family   = CAST(addressFamily);
    hints.ai_socktype = CAST(socketType);
    hints.ai_protocol = CAST(protocol);

    int ret = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &pAddress);
    if (ret)
    {
        ProcessSocketError(WSAGetLastError());
    }

    if (bind(socket, pAddress->ai_addr, pAddress->ai_addrlen))
    {
        ProcessSocketError(WSAGetLastError());
    }
}

void Socket::Listen(int backlog)
{
    Socket_
    if (listen(socket, 5))
    {
        ProcessSocketError(WSAGetLastError());
    }
}

void Socket::Connect(const std::string &ip, int port, AddressFamily addressFamily, SocketType socketType, Protocol protocol)
{
    Socket_ Open(addressFamily, socketType, protocol);

    addrinfo hints{};
    hints.ai_family   = CAST(addressFamily);
    hints.ai_socktype = CAST(socketType);
    hints.ai_protocol = CAST(protocol);

    int ret = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &pAddress);
    if (ret)
    {
        ProcessSocketError(WSAGetLastError());
    }

    if (connect(socket, pAddress->ai_addr, pAddress->ai_addrlen))
    {
        ProcessSocketError(WSAGetLastError());
    }
}

int Socket::ShutDown(SocketOperation operation)
{
    Socket_
    return shutdown(socket, int(operation));
}

void Socket::Close()
{
    Socket_
    int ret = closesocket(socket);
    if (ret)
    {
        ProcessSocketError(ret);
    }

    if (pAddress)
    {
        freeaddrinfo(pAddress);
        pAddress = nullptr;
    }

    socket = INVALID_SOCKET;
}

Socket *Socket::Accept()
{
    Socket_  SOCKADDR_IN6 address{};
    int length = sizeof(SOCKADDR_IN);

    if (pAddress->ai_family == int(AddressFamily::IPV6))
    {
        length = sizeof(SOCKADDR_IN6);
    }

    SOCKET acceptSocket = accept(socket, (SOCKADDR *)&address, &length);
    return acceptSocket != INVALID_SOCKET ? new Socket(Anonymize(acceptSocket)) : nullptr;
}

int Socket::Send(const char *buffer, int size)
{
    Socket_
    return send(socket, buffer, size, 0);
}

int Socket::Receive(char*buffer, int size)
{
    Socket_
    return recv(socket, buffer, size, 0);
}

std::string Socket::GetIpString() const
{
    Socket_ char ret[48] = {};

    if (pAddress->ai_family == int(AddressFamily::IPV6))
    {
        SOCKADDR_IN6 *addr = (SOCKADDR_IN6 *)pAddress->ai_addr;
        RtlIpv6AddressToStringA(&addr->sin6_addr, ret);
    }
    else
    {
        SOCKADDR_IN *addr = (SOCKADDR_IN *)pAddress->ai_addr;
        return inet_ntoa(addr->sin_addr);
    }

    return ret;
}

}

