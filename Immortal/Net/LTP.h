#pragma once

#include "Core.h"
#include "TCP.H"
#include "Interface/IObject.h"

namespace Immortal
{

#define LTP_VERSION "LTP/0.0.1"

enum class MessageType
{
    Request,
    Response,
};

static inline int EncodeHeader(char* dst, const std::string& key, const std::string& value)
{
    char* start = dst;

    memcpy(dst, key.data(), key.size());
    dst += key.size();

    dst[0] = ':';
    dst[1] = ' ';
    dst += 2;

    memcpy(dst, value.data(), value.size());
    dst += value.size();

    dst[0] = '\r';
    dst[1] = '\n';
    dst += 2;

    return dst - start;
}

#define LTP_REQUEST_HEADER "Request " LTP_VERSION "\r\n"

struct LightTransferProtocolMessage
{
    void AddHeader(const std::string &key, const std::string &value)
    {
        headers[key] = value;
    }

    std::string Serialize() const
    {
        char ret[4096] = {};

        auto ptr = ret;

        if (type == MessageType::Request)
        {
            memcpy(ptr, LTP_REQUEST_HEADER, 19);
            ptr += 19;
        }

        for (auto &header : headers)
        {
            ptr += EncodeHeader(ptr, header.first, header.second);
        }

        if (headers.find("payload-size") == headers.end() && !payload.empty())
        {
            ptr += EncodeHeader(ptr, "payload-size", std::to_string(payload.size()));
        }

        int padding = SLALIGN((uint64_t)ptr, 8) - (uint64_t)ptr;
        memset(ptr, '@', padding);
        ptr += padding;
        *(uint64_t*)ptr = *(uint64_t*)"!LTP-END";
        ptr += sizeof(uint64_t);
        *ptr = '\0';
        return ret + payload;
    }

    int version;

    MessageType type = MessageType::Request;

    std::unordered_map<std::string, std::string> headers;

    std::string payload;
};

using LTPM = LightTransferProtocolMessage;

class LTP : public TCP
{
public:
    int Send(const LTPM &message)
    {
        std::string bin = message.Serialize();
        return TCP::Send(bin.data(), bin.size());
    }

    LTP *Accept()
    {
        return (LTP *)Socket::Accept();
    }
};

}
