

#include "type_parser.h"
#include "content.h"
using namespace std;


void TypeParser::parseItem(Content& text, Message* message)
{
    variableName_ = text.readIdentifier();
    bool insertOK = message->variableNames.insert(variableName_).second;
    if (!insertOK) text.abortForParseError("variable name redefined");

    text.skipInvalidWords();
    if (text.cur() != '=') text.abortForParseError("lose =");
    text.next();
    text.skipInvalidWords();
    if (!isdigit(text.cur())) text.abortForParseError("digit");

    fieldNumber_ = text.readUInt<int>();
    insertOK = message->fieldNumbers.insert(fieldNumber_).second;
    if (!insertOK) text.abortForParseError("field number redefined");

    text.skipInvalidWords();
    if (text.cur() != ';')
    {
        if (text.cur() == '[')
        {
            text.next();
            text.skipInvalidWords();
            string word = text.readIdentifier();
            if (word == "default")
            {
                text.skipInvalidWords();
                if (text.cur() != '=') text.abortForParseError("lose =");
                text.next();
                defaultValue_ = text.readStringUtil("]");
                text.skipInvalidWords();
                if (text.cur() != ']') text.abortForParseError("lose ]");
                text.next();
                text.skipInvalidWords();
                if (text.cur() != ';') text.abortForParseError("lose ;");
            }
            else
            {
                text.abortForParseError("unknown keyword");
            }
        }
        else
        {
            text.abortForParseError("lose ;");
        }
    }
    text.next();
}

class UIntParser: public TypeParser
{
public:
    string wireType() const override { return "VARINT"; }

    string codeForByteSize(const string& variableName) const override
    { return "net::sizeofUInt(" + variableName + ")"; }
    string codeForWriter(const string& variableName) const override
    { return "out.writeUInt(" + variableName + ");"; }
    string codeForReader(const string& variableName) const override
    { return "in.readUInt(&" + variableName + ");"; }

};

class UInt64Parser: public UIntParser
{
    string realType() const override { return "uint64_t"; }
};

class UInt32Parser: public UIntParser
{
    string realType() const override { return "uint32_t"; }
};

class UInt16Parser: public UIntParser
{
    string realType() const override { return "uint16_t"; }
};

class UInt8Parser: public UIntParser
{
    string realType() const override { return "uint8_t"; }
};



class IntParser: public TypeParser
{
public:
    string wireType() const override { return "VARINT"; }

    string codeForByteSize(const string& variableName) const override
    { return "net::sizeofInt(" + variableName + ")"; }
    string codeForWriter(const string& variableName) const override
    { return "out.writeInt(" + variableName + ");"; }
    string codeForReader(const string& variableName) const override
    { return "in.readInt(&" + variableName + ");"; }

};

class Int64Parser: public IntParser
{
    string realType() const override { return "int64_t"; }
};

class Int32Parser: public IntParser
{
    string realType() const override { return "int32_t"; }
};

class Int16Parser: public IntParser
{
    string realType() const override { return "int16_t"; }
};

class Int8Parser: public IntParser
{
    string realType() const override { return "int8_t"; }
};

class Fixed64Parser: public TypeParser
{
public:
    string realType() const override { return "int64_t"; }
    string wireType() const override { return "FIXED64"; }

    string codeForByteSize(const string&) const override
    { return "8"; }
    string codeForWriter(const string& variableName) const override
    { return "out.writeFixed64(" + variableName + ");"; }
    string codeForReader(const string& variableName) const override
    { return "in.readFixed64(&" + variableName + ");"; }
};

class UFixed64Parser: public Fixed64Parser
{
    string realType() const override { return "uint64_t"; }
};

class DoubleParser: public Fixed64Parser
{
    string realType() const override { return "double"; }
};

class Fixed32Parser: public TypeParser
{
public:
    string realType() const override { return "int32_t"; }
    string wireType() const override { return "FIXED32"; }

    string codeForByteSize(const string&) const override
    { return "4"; }
    string codeForWriter(const string& variableName) const override
    { return "out.writeFixed32(" + variableName + ");"; }
    string codeForReader(const string& variableName) const override
    { return "in.readFixed32(&" + variableName + ");"; }
};

class UFixed32Parser: public Fixed32Parser
{
    string realType() const override { return "uint32_t"; }
};

class FloatParser: public Fixed32Parser
{
    string realType() const override { return "float"; }
};

class StringParser: public TypeParser
{
public:
    string realType() const override { return "std::string"; }

    string codeForByteSize(const string& variableName) const override
    { return "net::sizeofString(" + variableName + ")"; }
    string codeForWriter(const string& variableName) const override
    { return "out.writeString(" + variableName + ");"; }
    string codeForReader(const string& variableName) const override
    { return "in.readString(&" + variableName + ");"; }
};


class VectorParser: public TypeParser
{
public:
    VectorParser()
    {
        useCachedSize_ = true;
    }

    vector<string> includeList() const override { return {"<vector>", "<numeric>"}; }

    string realType() const override
    {
        return "std::vector<" + innerTypes_[0]->realType() + ">";
    }

    string codeForByteSize(const string& variableName) const override
    {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "std::accumulate(%s.cbegin(), %s.cend(), net::sizeofUInt(%s.size()),\n"
                 "    [](int sum, const %s& x) { return sum + %s; })",
                 variableName.c_str(), variableName.c_str(), variableName.c_str(),
                 innerTypes_[0]->realType().c_str(), innerTypes_[0]->codeForByteSize("x").c_str());
        return buf;
    }

    string codeForWriter(const string& variableName) const override
    {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "// writeVector\n"
                 "out.writeUInt(%s_cachedSize_);\n"
                 "out.writeUInt(%s.size());\n"
                 "for (const auto& x: %s) %s\n",
                 variableName.c_str(),
                 variableName.c_str(),
                 variableName.c_str(), innerTypes_[0]->codeForWriter("x").c_str());
        return buf;
    }

    string codeForReader(const string& variableName) const override
    {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "int len = in.readLength();\n"
                 "const uint8_t* end = in.cur() + len;\n"
                 "%s.clear();\n"
                 "%s.reserve(in.readUInt<unsigned int>());\n"
                 "while (in.cur() != end)\n"
                 "{\n"
                 "    %s x;\n"
                 "    %s;\n"
                 "    %s.push_back(std::move(x));\n"
                 "}",
                 variableName.c_str(),
                 variableName.c_str(),
                 innerTypes_[0]->realType().c_str(),
                 innerTypes_[0]->codeForReader("x").c_str(),
                 variableName.c_str());
        return buf;
    }

};


REGISTER_TYPE_PARSER(uint64, UInt64Parser)
REGISTER_TYPE_PARSER(uint32, UInt32Parser)
REGISTER_TYPE_PARSER(uint16, UInt16Parser)
REGISTER_TYPE_PARSER(uint8, UInt8Parser)
REGISTER_TYPE_PARSER(bool, UInt8Parser)

REGISTER_TYPE_PARSER(int64, Int64Parser)
REGISTER_TYPE_PARSER(int32, Int32Parser)
REGISTER_TYPE_PARSER(int16, Int16Parser)
REGISTER_TYPE_PARSER(int8, Int8Parser)

REGISTER_TYPE_PARSER(double, DoubleParser)
REGISTER_TYPE_PARSER(ufixed64, UFixed64Parser)
REGISTER_TYPE_PARSER(fixed64, Fixed64Parser)

REGISTER_TYPE_PARSER(float, FloatParser)
REGISTER_TYPE_PARSER(ufixed32, UFixed32Parser)
REGISTER_TYPE_PARSER(fixed32, Fixed32Parser)

REGISTER_TYPE_PARSER(string, StringParser)
REGISTER_TYPE_PARSER(vector, VectorParser)

