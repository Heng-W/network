
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>
#include "net_ext/message/coded_stream.h"
using namespace net;

struct EntryMessage : Message
{
    MESSAGE_TAG(0)

    std::pair<std::string, std::string> entry;

    EntryMessage() = default;
    EntryMessage(const std::pair<std::string, std::string>& _entry) : entry(_entry) {}

    int byteSize() const override
    { return sizeofString(1, entry.first) + sizeofString(2, entry.second);  }

    int encodeToBytes(char* buf) const override
    {
        CodedStreamWriter out(buf);
        out.writeString(1, entry.first);
        out.writeString(2, entry.second);
        return out.size();
    }

    bool decodeFromBytes(const char* buf, int len) override
    {
        CodedStreamReader in(buf, buf + len);
        while (in)
        {
            int tag = in.readTag();
            switch (fieldNumberOfTag(tag))
            {
                case 1:
                    entry.first = in.readString();
                    break;
                case 2:
                    entry.second = in.readString();
                    break;
                default:
                    return false;
            }
        }
        return true;
    }
};


template <class T>
class Required
{
public:
    using ValueType = T;

    Required(): isSet_(false) {}
    Required(const T& val): value_(val), isSet_(true) {}
    Required(T&& val): value_(std::move(val)), isSet_(true) {}

    operator T() const { return value_; }

    bool isSet() const { return isSet_; }

private:
    T value_;
    bool isSet_;
};

struct UserMessage : Message
{
    MESSAGE_TAG(1)

    Required<int64_t> userId;
    std::string userCode;
    std::string userName;
    std::string email;
    int age = 0;
    std::vector<std::string> roles;
    EntryMessage entry;
    std::map<std::string, std::string> address;


    int byteSize() const override
    {
        int len =  sizeofUInt(1, (int64_t)userId) + sizeofString(2, userCode) + sizeofString(3, userName) + sizeofString(4, email) + sizeofUInt(5, age);
        for (const auto& x : roles) len += sizeofString(6, x);
        for (const auto& x : address) len += sizeofMessage(7, EntryMessage(x));
        len += sizeofMessage(8, entry);
        return len;
    }

    int encodeToBytes(char* buf) const override
    {
        CodedStreamWriter out(buf);
        if (!userId.isSet())
        {
            std::cout << "require userId" << std::endl;
            abort();
        }
        out.writeUInt(1, (int64_t)userId);
        out.writeString(2, userCode);
        out.writeString(3, userName);
        out.writeString(4, email);
        out.writeUInt(5, age);
        for (const auto& x : roles) out.writeString(6, x);

        out.writeMessage(8, entry);

        for (const auto& x : address)
        {
            EntryMessage msg(x);
            msg.calcByteSize();
            out.writeMessage(7, msg);
        }

        return out.size();
    }

    bool decodeFromBytes(const char* buf, int len) override
    {
        CodedStreamReader in(buf, buf + len);
        while (in)
        {
            int tag = in.readTag();

            // std::cout << fieldNumberOfTag(tag) << std::endl;
            switch (fieldNumberOfTag(tag))
            {
                case 1:
                    userId = in.readUInt<int64_t>();
                    break;
                case 2:
                    userCode = in.readString();
                    break;
                case 3:
                    userName = in.readString();
                    break;
                case 4:
                    email = in.readString();
                    break;
                case 5:
                    in.readUInt(&age);
                    break;
                case 6:
                    roles.push_back(in.readString());
                    break;
                case 7:
                {
                    EntryMessage msg;
                    in.readMessage(&msg);
                    address.insert(std::move(msg.entry));
                    break;
                }
                case 8:
                    in.readMessage(&entry);
                    break;
                default:
                    return false;
            }
        }
        return true;
    }

};

int main()
{
    UserMessage msg;
    msg.userId = -1;
    msg.userCode = "00001";
    msg.userName = "cc";
    msg.age = 20;
    msg.roles = { "admin", "cc"};
    msg.address = {{"a", "1"}, {"b", "2"}};
    msg.entry.entry.first = "first";
    msg.entry.entry.second = "second";

    std::vector<char> buf(msg.calcByteSize());
    int len = msg.encodeToBytes(&*buf.begin());
    if (len != (int)buf.size())
    {
        std::cout << len << " " << buf.size() << std::endl;
        return -1;
    }
    for (size_t i = 0; i < buf.size(); ++i)
    {
        printf("%d ", buf[i]);
    }
    printf("\n\n");
    // [8, 2, 18, 5, 48, 48, 48, 48, 49, 26, 2, 99, 99, 40, 20, 50,
    // 5, 97, 100, 109, 105, 110, 50, 2, 99, 99, 58, 6, 10, 1, 97, 18,
    // 1, 49, 58, 6, 10, 1, 98, 18, 1, 50]

    UserMessage user2;
    bool ret = user2.decodeFromBytes(&*buf.cbegin(), buf.size());
    if (!ret)
    {
        std::cout << "parse fail" << std::endl;
        return -1;
    }
    std::cout << user2.userId << std::endl;
    std::cout << user2.userCode << std::endl;
    std::cout << user2.userName << std::endl;
    std::cout << user2.email << std::endl;
    std::cout << user2.age << std::endl;
    for (const auto& x : user2.roles) std::cout << x << "  ";
    std::cout << std::endl;
    for (const auto& x : user2.address) std::cout << "(" << x.first << "," << x.second << ")" << "  ";
    std::cout << std::endl;
    std::cout << user2.entry.entry.first << " " << user2.entry.entry.second << std::endl;
    return 0;
}
