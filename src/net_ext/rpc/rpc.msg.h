// auto-generated head file
#ifndef NET_RPC_MSG_H
#define NET_RPC_MSG_H

#include "net_ext/message/coded_stream.h"

namespace net
{

struct RpcRequest : net::Message
{
    MESSAGE_TAG(net_RpcRequest)

    uint64_t id;
    std::string method;
    std::string content;

    int encodeToBytes(char* buf) const override
    {
        net::CodedStreamWriter out(buf);
        out.writeTag(1, net::WireType::FIXED64);
        out.writeFixed64(id);
        out.writeTag(2, net::WireType::LENGTH_DELIMITED);
        out.writeString(method);
        out.writeTag(3, net::WireType::LENGTH_DELIMITED);
        out.writeString(content);
        return out.size();
    }

    bool decodeFromBytes(const char* buf, int len) override
    {
        net::CodedStreamReader in(buf, buf + len);
        while (in)
        {
            int tag = in.readTag();
            switch (net::fieldNumberOfTag(tag))
            {
                case 1:
                {
                    in.readFixed64(&id);                
                    break;
                }
                case 2:
                {
                    in.readString(&method);                
                    break;
                }
                case 3:
                {
                    in.readString(&content);                
                    break;
                }
                default:
                    if (!in.readUnknownField(tag)) return false;
                    break;
            }
        }
        return true;
    }

protected:
    int byteSize() const override
    {
        return net::sizeofTag(1) + 8 + 
               net::sizeofTag(2) + net::sizeofString(method) + 
               net::sizeofTag(3) + net::sizeofString(content);
    }

};

struct RpcResponse : net::Message
{
    MESSAGE_TAG(net_RpcResponse)

    uint64_t id;
    int32_t status;
    std::string describe;
    std::string content;

    int encodeToBytes(char* buf) const override
    {
        net::CodedStreamWriter out(buf);
        out.writeTag(1, net::WireType::FIXED64);
        out.writeFixed64(id);
        out.writeTag(2, net::WireType::VARINT);
        out.writeInt(status);
        out.writeTag(3, net::WireType::LENGTH_DELIMITED);
        out.writeString(describe);
        out.writeTag(4, net::WireType::LENGTH_DELIMITED);
        out.writeString(content);
        return out.size();
    }

    bool decodeFromBytes(const char* buf, int len) override
    {
        net::CodedStreamReader in(buf, buf + len);
        while (in)
        {
            int tag = in.readTag();
            switch (net::fieldNumberOfTag(tag))
            {
                case 1:
                {
                    in.readFixed64(&id);                
                    break;
                }
                case 2:
                {
                    in.readInt(&status);                
                    break;
                }
                case 3:
                {
                    in.readString(&describe);                
                    break;
                }
                case 4:
                {
                    in.readString(&content);                
                    break;
                }
                default:
                    if (!in.readUnknownField(tag)) return false;
                    break;
            }
        }
        return true;
    }

protected:
    int byteSize() const override
    {
        return net::sizeofTag(1) + 8 + 
               net::sizeofTag(2) + net::sizeofInt(status) + 
               net::sizeofTag(3) + net::sizeofString(describe) + 
               net::sizeofTag(4) + net::sizeofString(content);
    }

};

} // namespace net

#endif // NET_RPC_MSG_H

