// 二进制协议的打包解包
#ifndef NET_CODED_STREAM_H
#define NET_CODED_STREAM_H

#include <assert.h>
#include <string>
#include <algorithm>
#include "net/endian.h"
#include "message.h"

namespace net
{

enum class WireType
{
    VARINT = 0, // 可变长整数
    FIXED64 = 1, // 固定64位，如double
    LENGTH_DELIMITED = 2, // 字节流，字符串等
    FIXED32 = 5 // 固定32位，如float
};

constexpr int fieldNumberOfTag(unsigned int tag) { return tag >> 3; }
constexpr WireType wireTypeOfTag(unsigned int tag) { return static_cast<WireType>(tag & 0x07); }

template <typename T>
inline int sizeofUInt(const T& value)
{
    static_assert(std::is_integral<T>::value, "should be integral");
    using unsignedType = typename std::make_unsigned<T>::type;
    unsignedType val = value;
    int num = 1;
    while (val >>= 7) ++num;
    return num;
}

template <typename T>
inline int sizeofInt(const T& value)
{ return sizeofUInt((value << 1) ^ (value >> (8 * sizeof(value) - 1))); }

inline int sizeofByteArray(int len)
{ return sizeofUInt(len) + len; }

inline int sizeofString(const std::string& str)
{ return sizeofByteArray(str.size()); }

inline int sizeofMessage(const Message& message)
{ return sizeofByteArray(message.calcByteSize()); }

inline int sizeofTag(int fieldNumber)
{ return sizeofUInt(fieldNumber << 3); }


// 写入编码字节流
class CodedStreamWriter
{
public:
    CodedStreamWriter(void* buf)
        : cur_(static_cast<uint8_t*>(buf)),
          begin_(cur_)
    {}

    // 将整数压缩成字符数组
    template <typename T>
    void writeUInt(const T& value)
    {
        static_assert(std::is_integral<T>::value, "should be integral");
        using unsignedType = typename std::make_unsigned<T>::type;
        unsignedType val = value;
        do
        {
            *cur_ = static_cast<uint8_t>(val & 0x7f);
            val >>= 7;
            if (val) *cur_ |= 0x80; // 标识是否结束（1:未结束，0:结束）
            ++cur_;
        }
        while (val);
    }

    // ZigZag编码
    template <typename T>
    void writeInt(const T& value)
    {
        static_assert(std::is_integral<T>::value, "should be integral");
        writeUInt((value << 1) ^ (value >> (8 * sizeof(value) - 1)));
    }

    template <typename T>
    void writeFixed64(const T& value)
    {
        static_assert(sizeof(T) == 8, "should be 64 bits");
        *reinterpret_cast<uint64_t*>(cur_) = hostToNetwork64(*reinterpret_cast<const uint64_t*>(&value));
        cur_ += sizeof(T);
    }

    template <typename T>
    void writeFixed32(const T& value)
    {
        static_assert(sizeof(T) == 4, "should be 32 bits");
        *reinterpret_cast<uint32_t*>(cur_) = hostToNetwork32(*reinterpret_cast<const uint32_t*>(&value));
        cur_ += sizeof(T);
    }

    void writeByteArray(const void* data, int len)
    {
        writeUInt(len);
        cur_ = std::copy_n(static_cast<const uint8_t*>(data), len, cur_);
    }

    void writeString(const std::string& str)
    {
        writeByteArray(str.data(), str.size());
    }

    void writeMessage(const Message& message)
    {
        int byteSize = message.cachedSize(); // 使用已缓存的序列化大小
        writeUInt(byteSize);
        int nwrote = message.encodeToBytes(reinterpret_cast<char*>(cur_));
        (void)nwrote;
        assert(byteSize == nwrote);
        cur_ += byteSize;
    }

    template <typename T>
    void writeUIntWithTag(int fieldNumber, const T& value)
    {
        writeTag(fieldNumber, WireType::VARINT);
        writeUInt(value);
    }

    template <typename T>
    void writeIntWithTag(int fieldNumber, const T& value)
    {
        writeTag(fieldNumber, WireType::VARINT);
        writeInt(value);
    }

    template <typename T>
    void writeFixed64WithTag(int fieldNumber, const T& value)
    {
        writeTag(fieldNumber, WireType::FIXED64);
        writeFixed64(value);
    }

    template <typename T>
    void writeFixed32WithTag(int fieldNumber, const T& value)
    {
        writeTag(fieldNumber, WireType::FIXED32);
        writeFixed32(value);
    }

    void writeByteArrayWithTag(int fieldNumber, const void* data, int len)
    {
        writeTag(fieldNumber, WireType::LENGTH_DELIMITED);
        writeByteArray(data, len);
    }

    void writeStringWithTag(int fieldNumber, const std::string& str)
    {
        writeTag(fieldNumber, WireType::LENGTH_DELIMITED);
        writeString(str);
    }

    void writeMessageWithTag(int fieldNumber, const Message& message)
    {
        writeTag(fieldNumber, WireType::LENGTH_DELIMITED);
        writeMessage(message);
    }

    void writeTag(int fieldNumber, WireType wireType)
    { writeUInt(makeTag(fieldNumber, wireType)); }

    int size() const { return cur_ - begin_; }
    const uint8_t* cur() const { return cur_; }

private:
    static constexpr int makeTag(int fieldNumber, WireType wireType)
    { return (fieldNumber << 3) | static_cast<int>(wireType); }

    uint8_t* cur_;
    const uint8_t* begin_;
};


// 读取编码字节流
class CodedStreamReader
{
public:
    CodedStreamReader(const void* begin, const void* end)
        : cur_(static_cast<const uint8_t*>(begin)),
          end_(static_cast<const uint8_t*>(end))
    {}

    // 将字符数组还原成整数
    template <typename T>
    void readUInt(T* value)
    {
        static_assert(std::is_integral<T>::value, "should be integral");
        assert(end_ - cur_ > 0);
        *value = 0;
        int bitCount = 0;
        uint8_t c;
        do
        {
            c = *cur_++;
            *value |= (c & 0x7f) << bitCount;
            bitCount += 7;
        }
        while (c & 0x80);
    }

    template <typename T>
    void readInt(T* value)
    {
        using unsignedType = typename std::make_unsigned<T>::type;
        unsignedType n;
        readUInt(&n);
        *value = (n >> 1) ^ -(n & 1);
    }

    template <typename T>
    void readFixed64(T* value)
    {
        static_assert(sizeof(T) == 8, "should be 64 bits");
        assert(end_ - cur_ >= static_cast<int>(sizeof(T)));
        *reinterpret_cast<uint64_t*>(value) = networkToHost64(*reinterpret_cast<const uint64_t*>(cur_));
        cur_ += sizeof(T);
    }

    template <typename T>
    void readFixed32(T* value)
    {
        static_assert(sizeof(T) == 4, "should be 32 bits");
        assert(end_ - cur_ >= static_cast<int>(sizeof(T)));
        *reinterpret_cast<uint32_t*>(value) = networkToHost32(*reinterpret_cast<const uint32_t*>(cur_));
        cur_ += sizeof(T);
    }

    // 字节流读取到buf
    void readByteArray(void* buf, int len)
    {
        std::copy_n(cur_, len, static_cast<uint8_t*>(buf));
        cur_ += len;
    }

    void readString(std::string* str)
    {
        int len = readLength();
        *str = std::string(reinterpret_cast<const char*>(cur_), len);
        cur_ += len;
    }

    bool readMessage(Message* message)
    {
        int len = readLength();
        bool ret = message->decodeFromBytes(reinterpret_cast<const char*>(cur_), len);
        cur_ += len;
        return ret;
    }

    bool readUnknownField(int tag)
    {
        switch (wireTypeOfTag(tag))
        {
            case WireType::VARINT:
                while (*cur_++ & 0x80);
                break;
            case WireType::FIXED64:
                cur_ += 8;
                break;
            case WireType::FIXED32:
                cur_ += 4;
                break;
            case WireType::LENGTH_DELIMITED:
            {
                int len = readLength();
                cur_ += len;
                break;
            }
            default:
                cur_ = end_;
                return false;
        }
        return true;
    }

    template <typename T>
    T readUInt()
    {
        T value;
        readUInt(&value);
        return value;
    }

    template <typename T>
    T readInt()
    {
        T value;
        readInt(&value);
        return value;
    }

    template <typename T>
    T readFixed64()
    {
        T value;
        readFixed64(&value);
        return value;
    }

    template <typename T>
    T readFixed32()
    {
        T value;
        readFixed32(&value);
        return value;
    }

    std::string readString()
    {
        std::string res;
        readString(&res);
        return res;
    }

    template <typename T>
    T readMessage()
    {
        T message;
        readMessage(&message);
        return message;
    }

    int peekLength()
    {
        const uint8_t* pos = cur_;
        int len = readLength();
        cur_ = pos;
        return len;
    }

    int readTag() { return readUInt<int>(); }
    int readLength() { return readUInt<int>(); }

    operator bool() const { return cur_ != end_; }

    const uint8_t* cur() const { return cur_; }
    const uint8_t* end() const { return end_; }

private:
    const uint8_t* cur_;
    const uint8_t* end_;
};

} // namespace net

#endif // NET_CODED_STREAM_H
