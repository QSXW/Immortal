#pragma once

#include "sl.h"

#ifdef WINDOWS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#if defined( _MSC_VER )
#pragma comment(lib, "ws2_32.lib")
#endif
#endif

namespace Immortal
{

enum Family
{
    Ipv4       = AF_INET,
    Ipv6       = AF_INET6,
    Unspecifed = AF_UNSPEC
};

enum SocketType
{
    Stream = SOCK_STREAM
};
        
enum Protocol
{
    TCP = IPPROTO_TCP,
    UDP = IPPROTO_UDP
};

enum SocketStatus
{
    Invalid = INVALID_SOCKET,
    Error   = SOCKET_ERROR
};

class AddrInfo
{
public:
    AddrInfo() :
        response{ nullptr }
    {
        CleanUpObject(&hints);
    }

    AddrInfo(const int addressFamily, const int socketType, const int protocol) :
        response{ nullptr }
    {
        CleanUpObject(&hints);
        hints.ai_family   = addressFamily;
        hints.ai_socktype = socketType;
        hints.ai_protocol = protocol;
    }
    
    ~AddrInfo()
    {
        freeaddrinfo(response);
    }

    int Get(const std::string &ip, const std::string &port)
    {
        return getaddrinfo(ip.c_str(), port.c_str(), &hints, &response);
    }

    int Connect(SOCKET socket)
    {
        return connect(socket, response->ai_addr, response->ai_addrlen);
    }

    int Close(SOCKET socket)
    {
        return closesocket(socket);
    }

private:
    struct addrinfo *response;
    struct addrinfo  hints;
};

class Socket
{
public:
    enum class Error
    {
        Running,
        Closed,
        NotInitialized,
        NotRecoverable,
        Undefined = ~0
    };

    enum Status : SOCKET
    {
        Invalid = INVALID_SOCKET,
        Fault   = SOCKET_ERROR
    };

    std::map<uint32_t, Error> InternalErrorMap = {
        { WSANOTINITIALISED, Error::NotInitialized },
        { WSANO_RECOVERY,    Error::NotRecoverable }
    };

public:
    Socket() :
        handle{ Status::Invalid }
    {

    }

    Socket(const std::string &ip, const std::string &port, const int addressFamily = Family::Ipv4, const int socketType = SocketType::Stream, const int protocol = Protocol::TCP) :
        handle{ Status::Invalid },
        addrInfo{ addressFamily, socketType, protocol }
    {
        if (WSAStartup(MAKEWORD(2,2), &wsaData))
        {
            return;
        }
        if (addrInfo.Get(ip, port))
        {
            goto cleanup;
        }

        handle = socket(addressFamily, socketType, protocol);
        if (handle == Status::Invalid)
        {
            goto cleanup;
        }

        if (addrInfo.Connect(*this) == Status::Fault)
        {
            addrInfo.Close(*this);
            goto cleanup;
        }
        return;
    cleanup:
        WSACleanup();
    }

    Socket(const std::string &ip, const int port, const int addressFamily = Family::Ipv4, const int socketType = SocketType::Stream, const int protocol = Protocol::TCP)
        : Socket{ ip, std::to_string(port), addressFamily, socketType, protocol }
    {

    }

    ~Socket()
    {
        WSACleanup();
    }

    operator SOCKET&()
    {
        return handle;
    }

    operator SOCKET() const
    {
        return handle;
    }

    Error GetLastError()
    {
        auto it = InternalErrorMap.find(::GetLastError());
        if (it == InternalErrorMap.end())
        {
            return Error::Undefined;
        }
        return it->second;
    }

    bool Readable()
    {
        return !(handle == Status::Invalid);
    }

    template <class T>
    int Send(const std::vector<T> &data)
    {
        return send(handle, reinterpret_cast<const char *>(data.data()), data.size(), 0);
    }

    template <class T>
    int Send(const T *data, size_t size)
    {
        return send(handle, reinterpret_cast<const char *>(data), size, 0);
    }

    template <class T>
    int Receive(T *out, size_t size)
    {
        return recv(handle, reinterpret_cast<char *>(out), sizeof(T) * size, 0);
    }

    template <class T>
    int Receive(std::vector<T> *out)
    {
        int status = 0;
        std::vector<char> buffer;
        buffer.resize(65536);

        while(true)
        {
            status = Receive(buffer.data(), buffer.size());
            if (status > 0)
            {
                out->resize(out->size() + status);
                std::copy(buffer.begin(), buffer.begin() + status, out->end() - status);
            }
            else
            {
                break;
            }
        };

        return status;
    }

    template <class Callback>
    int Receive(Callback callback)
    {
        int status = 0;
        std::vector<char> buffer;
        buffer.resize(65536);

        while (true)
        {
            status = Receive(buffer.data(), buffer.size());
            if (status > 0)
            {
                callback(buffer, status);
            }
            else
            {
                break;
            }
        };

        return status;
    }

private:
    SOCKET    handle;
    WSAData   wsaData;
    AddrInfo  addrInfo;
};

}
