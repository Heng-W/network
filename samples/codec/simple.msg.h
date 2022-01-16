
#include "net_ext/message/message.h"

struct SimpleMessage : net::Message
{
    MESSAGE_TAG(0)
    
    char value;
    
    int byteSize() const override { return 1; }
    
    int encodeToBytes(char* buf) const override
    {
        *buf = value;
        return 1;
    }
    
    bool decodeFromBytes(const char* buf, int ) override
    {
        value = *buf;
        return true;
    }
    
};
