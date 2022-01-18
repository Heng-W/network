#ifndef TYPE_PARSER_H
#define TYPE_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

struct Content;
struct Message;

class TypeParser
{
public:

    enum TypeFlag { kOptional, kRepeated, kRequired };

    virtual ~TypeParser() = default;

    virtual std::vector<std::string> includeList() const { return {}; }
    virtual std::string realType() const { return typeName_; }

    virtual std::string codeForByteSize(const std::string& variableName) const = 0;
    virtual std::string codeForWriter(const std::string& variableName) const = 0;
    virtual std::string codeForReader(const std::string& variableName) const = 0;

    virtual std::string wireType() const { return "LENGTH_DELIMITED"; }

    std::string codeForByteSize() const
    {
        std::string res = "net::sizeofTag(" + std::to_string(fieldNumber_) + ") + ";
        if (useCachedSize_)
            return variableName_ + "_cachedSize_ = " + codeForByteSize(variableName_) + ";";
        return res + codeForByteSize(variableName_);
    }

    std::string codeForCachedSize() const
    {
        std::string res = "net::sizeofTag(" + std::to_string(fieldNumber_) + ") + ";
        return res + "net::sizeofByteArray(" + variableName_ + "_cachedSize_)";
    }

    std::string codeForWriter() const
    {
        return "out.writeTag(" + std::to_string(fieldNumber_) + ", net::WireType::" + wireType() + ");\n" +
               codeForWriter(variableName_);
    }

    std::string codeForReader() const
    { return codeForReader(variableName_); }

    void setTypeName(const std::string& name) { typeName_ = name; }
    void setInnerTypes(const std::vector<std::shared_ptr<TypeParser>>& type)
    { innerTypes_ = type; }


    void parseItem(Content& text, Message*);

    bool useCachedSize() const { return useCachedSize_; }

    int fieldNumber() const { return fieldNumber_; }
    const std::string& variableName() const { return variableName_; }
    const std::string& defaultValue() const { return defaultValue_; }

protected:

    std::string typeName_;
    std::vector<std::shared_ptr<TypeParser>> innerTypes_;
    std::string variableName_;
    int fieldNumber_;
    std::string defaultValue_;

    bool useCachedSize_ = false;
};

using TypeParserPtr = std::shared_ptr<TypeParser>;

class DefaultParser: public TypeParser
{
public:
    std::string codeForByteSize(const std::string& variableName) const override
    { return "net::sizeofMessage(" + variableName + ")"; }
    std::string codeForWriter(const std::string& variableName) const override
    { return "out.writeMessage(" + variableName + ");"; }
    std::string codeForReader(const std::string& variableName) const override
    { return "in.readMessage(&" + variableName + ");"; }
};

class TypeParserFactory
{
public:
    TypeParserPtr create(const std::string& typeName)
    {
        auto it = parserMap_.find(typeName);
        TypeParserPtr parser = it == parserMap_.end()
                               ? std::make_shared<DefaultParser>() : it->second();
        parser->setTypeName(typeName);
        return parser;
    }

    template <class T>
    bool registerParser(const std::string& typeName)
    {
        static_assert(std::is_base_of<TypeParser, T>::value,
                      "T must be derived from TypeParser.");
        return parserMap_.insert({typeName, []{ return std::make_shared<T>(); }}).second;
    }

    static TypeParserFactory& instance()
    {
        static TypeParserFactory _instance;
        return _instance;
    }

private:
    TypeParserFactory() = default;

    std::map<std::string, std::function<TypeParserPtr()>> parserMap_;

};


#define REGISTER_TYPE_PARSER(typeName, TypeParserClassName) \
    static bool s_registerParserOf ## typeName = [] \
            {  \
               return TypeParserFactory::instance() \
                      .registerParser<TypeParserClassName>(#typeName); \
            }();

#endif // TYPE_PARSER_H
