/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include "Socket.h"

namespace Immortal
{

class TCP : public Socket
{
public:
    void Connect(const std::string &ip, int port, AddressFamily addressFamily = AddressFamily::IPV4)
    {
        Socket::Connect(ip, port, addressFamily, SocketType::Stream, Protocol::TCP);
    }

    void Bind(const std::string &ip, int port, AddressFamily addressFamily = AddressFamily::IPV4)
    {
        Socket::Bind(ip, port, addressFamily, SocketType::Stream, Protocol::TCP);
    }

    void Listen(int backlog)
    {
        Socket::Listen(backlog);
    }
};

}
