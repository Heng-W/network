

#include "type_parser.h"
using namespace std;

class HashMapParser: public TypeParser
{
public:
    HashMapParser()
    {
        useCachedSize_ = true;
    }

    vector<string> includeList() const override { return {"<unordered_map>", "<numeric>"}; }

    string realType() const override
    {
        return "std::unordered_map<" + innerTypes_[0]->realType() + ", " + innerTypes_[1]->realType() + ">";
    }

    string codeForByteSize(const string& variableName) const override
    {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "std::accumulate(%s.cbegin(), %s.cend(), net::sizeofUInt(%s.size()),\n"
                 "    [](int sum, const decltype(%s)::value_type& x) { return sum + %s + %s; })",
                 variableName.c_str(), variableName.c_str(), variableName.c_str(),
                 variableName.c_str(),
                 innerTypes_[0]->codeForByteSize("x.first").c_str(),
                 innerTypes_[1]->codeForByteSize("x.second").c_str());
        return buf;
    }

    string codeForWriter(const string& variableName) const override
    {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "out.writeUInt(%s_cachedSize_);\n"
                 "out.writeUInt(%s.size());\n"
                 "for (const auto& x: %s)\n"
                 "{\n"
                 "%s\n"
                 "%s\n"
                 "}",
                 variableName.c_str(),
                 variableName.c_str(),
                 variableName.c_str(),
                 innerTypes_[0]->codeForWriter("x.first").c_str(),
                 innerTypes_[1]->codeForWriter("x.second").c_str());
        return buf;
    }

    string codeForReader(const string& variableName) const override
    {
        char buf[1024];
        snprintf(buf, sizeof(buf),
                 "int len = in.readLength();\n"
                 "const uint8_t* end = in.cur() + len;\n"
                 "%s.clear();\n"
                 "size_t n = in.readUInt<unsigned int>();\n"
                 "while (in.cur() != end)\n"
                 "{\n"
                 "    %s x;\n"
                 "    %s;\n"
                 "    %s y;\n"
                 "    %s;\n"
                 "    %s[std::move(x)] = std::move(y);\n"
                 "}\n"
                 "if(n != %s.size()) throw std::runtime_error(\"size not match\");",
                 variableName.c_str(),
                 innerTypes_[0]->realType().c_str(),
                 innerTypes_[0]->codeForReader("x").c_str(),
                 innerTypes_[1]->realType().c_str(),
                 innerTypes_[1]->codeForReader("y").c_str(),
                 variableName.c_str(),
                 variableName.c_str());
        return buf;
    }

};


REGISTER_TYPE_PARSER(hashmap, HashMapParser);
