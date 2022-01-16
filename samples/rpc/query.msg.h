// auto-generated head file
#ifndef TEST_QUERY_MSG_H
#define TEST_QUERY_MSG_H

#include "net_ext/message/coded_stream.h"
#include <vector>
#include <numeric>

namespace test
{

struct Query : net::Message
{
    MESSAGE_TAG(test_Query)

    int64_t id;
    std::string questioner;
    std::vector<int16_t> a;
    std::vector<std::string> question;
private:
    mutable int a_cachedSize_;
    mutable int question_cachedSize_;
public:

    int encodeToBytes(char* buf) const override
    {
        net::CodedStreamWriter out(buf);
        out.writeTag(1, net::WireType::VARINT);
        out.writeInt(id);
        out.writeTag(2, net::WireType::LENGTH_DELIMITED);
        out.writeString(questioner);
        out.writeTag(4, net::WireType::LENGTH_DELIMITED);
        // writeVector
        out.writeUInt(a_cachedSize_);
        out.writeUInt(a.size());
        for (const auto& x: a) out.writeInt(x);

        out.writeTag(3, net::WireType::LENGTH_DELIMITED);
        // writeVector
        out.writeUInt(question_cachedSize_);
        out.writeUInt(question.size());
        for (const auto& x: question) out.writeString(x);

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
                    in.readInt(&id);                
                    break;
                }
                case 2:
                {
                    in.readString(&questioner);                
                    break;
                }
                case 4:
                {
                    int len = in.readLength();
                    const uint8_t* end = in.cur() + len;
                    a.clear();
                    a.reserve(in.readUInt<unsigned int>());
                    while (in.cur() != end)
                    {
                        int16_t x;
                        in.readInt(&x);;
                        a.push_back(std::move(x));
                    }                
                    break;
                }
                case 3:
                {
                    int len = in.readLength();
                    const uint8_t* end = in.cur() + len;
                    question.clear();
                    question.reserve(in.readUInt<unsigned int>());
                    while (in.cur() != end)
                    {
                        std::string x;
                        in.readString(&x);;
                        question.push_back(std::move(x));
                    }                
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
        a_cachedSize_ = std::accumulate(a.cbegin(), a.cend(), net::sizeofUInt(a.size()),
            [](int sum, const int16_t& x) { return sum + net::sizeofInt(x); });
        question_cachedSize_ = std::accumulate(question.cbegin(), question.cend(), net::sizeofUInt(question.size()),
            [](int sum, const std::string& x) { return sum + net::sizeofString(x); });
        return // byte size
                net::sizeofTag(1) + net::sizeofInt(id) + 
                net::sizeofTag(2) + net::sizeofString(questioner) + 
                net::sizeofTag(4) + net::sizeofByteArray(a_cachedSize_) + 
                net::sizeofTag(3) + net::sizeofByteArray(question_cachedSize_);
    }

};

struct Answer : net::Message
{
    MESSAGE_TAG(test_Answer)

    struct Test : net::Message
    {
        MESSAGE_TAG(test_Answer_Test)

        int32_t id;

        int encodeToBytes(char* buf) const override
        {
            net::CodedStreamWriter out(buf);
            out.writeTag(1, net::WireType::VARINT);
            out.writeInt(id);
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
                        in.readInt(&id);                    
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
            return // byte size
                    net::sizeofTag(1) + net::sizeofInt(id);
        }

    };

    int64_t id;
    std::string questioner;
    std::string answerer;
    std::vector<std::string> solution;
private:
    mutable int solution_cachedSize_;
public:

    int encodeToBytes(char* buf) const override
    {
        net::CodedStreamWriter out(buf);
        out.writeTag(1, net::WireType::VARINT);
        out.writeInt(id);
        out.writeTag(2, net::WireType::LENGTH_DELIMITED);
        out.writeString(questioner);
        out.writeTag(3, net::WireType::LENGTH_DELIMITED);
        out.writeString(answerer);
        out.writeTag(4, net::WireType::LENGTH_DELIMITED);
        // writeVector
        out.writeUInt(solution_cachedSize_);
        out.writeUInt(solution.size());
        for (const auto& x: solution) out.writeString(x);

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
                    in.readInt(&id);                
                    break;
                }
                case 2:
                {
                    in.readString(&questioner);                
                    break;
                }
                case 3:
                {
                    in.readString(&answerer);                
                    break;
                }
                case 4:
                {
                    int len = in.readLength();
                    const uint8_t* end = in.cur() + len;
                    solution.clear();
                    solution.reserve(in.readUInt<unsigned int>());
                    while (in.cur() != end)
                    {
                        std::string x;
                        in.readString(&x);;
                        solution.push_back(std::move(x));
                    }                
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
        solution_cachedSize_ = std::accumulate(solution.cbegin(), solution.cend(), net::sizeofUInt(solution.size()),
            [](int sum, const std::string& x) { return sum + net::sizeofString(x); });
        return // byte size
                net::sizeofTag(1) + net::sizeofInt(id) + 
                net::sizeofTag(2) + net::sizeofString(questioner) + 
                net::sizeofTag(3) + net::sizeofString(answerer) + 
                net::sizeofTag(4) + net::sizeofByteArray(solution_cachedSize_);
    }

};

struct Empty : net::Message
{
    MESSAGE_TAG(test_Empty)

    int32_t id;

    int encodeToBytes(char* buf) const override
    {
        net::CodedStreamWriter out(buf);
        out.writeTag(1, net::WireType::VARINT);
        out.writeInt(id);
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
                    in.readInt(&id);                
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
        return // byte size
                net::sizeofTag(1) + net::sizeofInt(id);
    }

};

} // namespace test

#endif // TEST_QUERY_MSG_H

