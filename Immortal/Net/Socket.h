/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"

namespace Immortal
{

enum class AddressFamily
{
#ifdef _WIN32
    Unspecified   = 0,
    Unix          = 1,
    InterNetwork  = 2,
    ImpLink       = 3,
    PUP           = 4,
    Chaos         = 5,
    IPX           = 6,
    NS            = 6,
    ISO           = 7,
    OSI           = 7,
    ECMA          = 8,
    Datakit       = 9,
    CCITT         = 10,
    SNA           = 11,
    DECnet        = 12,
    DLI           = 13,
    LAT           = 14,
    HYLINK        = 15,
    AppleTalk     = 16,
    NetBios       = 17,
    VoiceView     = 18,
    FireFox       = 19,
    UNKNOWN1      = 20,
    Banyan        = 21,
    ATM           = 22,
    InterNetwork6 = 23,
    Cluster       = 24,
    _12844        = 25,
    IRDA          = 26,
    NETDES        = 28,
#else
    Unspecified   = 0,
    Unix          = 1,
    InterNetwork  = 2,
    InterNetwork6 = 10,
#endif
    IPV4          = InterNetwork,
    IPV6          = InterNetwork6,
};

enum class SocketType
{
    None,
    Stream,
    Datagram,
    Raw,
    ReliablyDeliveredMessage,
    SequencedPacketStream,
};

enum class Protocol
{
#ifdef _WIN32
    TCP,
    UDP,
#else
    TCP = 6,
    UDP = 17,
#endif
};

enum class SocketOperation
{
    Receive,
    Send,
    Both,
};

class SocketExecption : public RuntimeException
{
public:
    SocketExecption(const std::string &what) :
        RuntimeException{ "Socket::" + what }
    {

    }
};

struct SocketHandle
{
    uint8_t Reserved[64];
};

class Socket
{
public:
    Socket();

    Socket(Anonymous socket);

    ~Socket();

    operator Anonymous() const;

    operator bool() const;

    void Open(AddressFamily addressFamily, SocketType socketType, Protocol protocol);

    void Bind(const std::string &ip, int port, AddressFamily addressFamily, SocketType socketType, Protocol protocol);

    void Listen(int backlog);

    void Connect(const std::string &ip, int port, AddressFamily addressFamily, SocketType socketType, Protocol protocol);

    void Close();

    int ShutDown(SocketOperation operation);

    Socket *Accept();

    int Send(const char *buffer, int size);

    int Receive(char*buffer, int size);

    std::string GetIpString() const;

    template <class T>
    int Send(const T &data)
    {
        return Send((const char *)(data.data()), data.size());
    }

    template <class T>
    int Receive(T &out)
    {
        int status = 0;
        char buffer[4096];

        while(true)
        {
            status = Receive(buffer, SL_ARRAY_LENGTH(buffer));
            if (status <= 0)
            {
                break;
            }
            out.resize(out.size() + status);
            std::copy(buffer, buffer + status, out.end() - status);
        };

        return status;
    }

private:
    SocketHandle handle;
};

}
